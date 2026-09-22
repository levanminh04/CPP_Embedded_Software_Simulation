#include "traffic/Application.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace traffic
{

    Application::Application()
        : config_(Config::defaults()),
          input_(),
          sensor_(config_),
          timer_(config_.tickMs),
          controller_(config_),
          output_(),
          display_(),
          logger_("logs/system.log"),
          running_(true),
          lastCommandBuffer_()
    {
    }

    int Application::run()
    {
        if (!config_.isValid())
        {
            std::cerr
                << "ERROR: Invalid configuration.\n";

            return 1;
        }

        if (!logger_.isOpen())
        {
            std::cerr
                << "ERROR: Cannot open log file.\n";

            return 1;
        }

        logger_.logInfo(
            "Traffic controller started");

        // Hiển thị trạng thái STARTUP_ALL_RED ngay khi khởi động.
        render(input_.currentBuffer());

        while (running_)
        {
            bool shouldRender = false;

            // =================================================
            // 1. INPUT
            // =================================================

            Event event;

            if (input_.poll(event))
            {
                handleInputEvent(event);
                shouldRender = true;
            }

            // Nếu user vừa nhập Q thì không cần xử lý tiếp tick.
            if (!running_)
            {
                break;
            }

            // =================================================
            // 2. TIMER
            // =================================================

            const int elapsedTicks = timer_.consumeTicks();

            for (int i = 0; i < elapsedTicks; ++i)
            {
                const ControllerResult result =
                    controller_.tick();

                handleControllerResult(result);
                shouldRender = true;
            }

            // =================================================
            // 3. INPUT BUFFER DISPLAY
            // =================================================

            const std::string currentCommand =
                input_.currentBuffer();

            if (currentCommand != lastCommandBuffer_)
            {
                lastCommandBuffer_ = currentCommand;
                shouldRender = true;
            }

            // =================================================
            // 4. OUTPUT + DISPLAY
            // =================================================

            if (shouldRender)
            {
                render(currentCommand);
            }

            // Super loop không được chạy 100% CPU.
            std::this_thread::sleep_for(
                std::chrono::milliseconds(10));
        }

        logger_.logInfo(
            "Traffic controller stopped");

        return 0;
    }

    void Application::handleInputEvent(
        const Event &event)
    {
        // SENSOR_UPDATE không đưa thẳng vào Controller.
        // Application cập nhật Sensor trước.
        if (event.type == EventType::SENSOR_UPDATE)
        {
            handleSensorUpdate(event);
            return;
        }

        // P / E / Q / INVALID_INPUT đi vào Controller.
        const ControllerResult result =
            controller_.apply(event);

        handleControllerResult(result);
    }

    void Application::handleSensorUpdate(
        const Event &event)
    {
        // Sensor chịu trách nhiệm validate + classify.
        const SensorReading reading =
            sensor_.update(
                event.direction,
                event.vehicleCount);

        if (!reading.valid)
        {
            logger_.logError(
                "SENSOR_ERROR: invalid vehicle count = " +
                std::to_string(event.vehicleCount));

            // Controller hiện tại vẫn có updateSensor().
            // Gọi để giữ đúng contract code hiện tại.
            controller_.updateSensor(
                event.direction,
                reading);

            return;
        }

        logger_.logSensorReading(
            event.direction,
            event.vehicleCount);

        // QUAN TRỌNG:
        // Controller hiện tại vẫn giữ nsReading/ewReading riêng,
        // nên Application phải truyền reading sang Controller.
        //
        // Sau khi refactor Sensor ownership về sau,
        // dòng này có thể được bỏ.
        controller_.updateSensor(
            event.direction,
            reading);
    }

    void Application::handleControllerResult(
        const ControllerResult &result)
    {
        // =================================================
        // LOG EVENT
        // =================================================

        switch (result.action)
        {
        case ControllerAction::PEDESTRIAN_REQUEST_RECORDED:
            logger_.logPedestrianRequest();
            break;

        case ControllerAction::EMERGENCY_ON_REQUESTED:
            logger_.logInfo(
                "Emergency ON requested");
            break;

        case ControllerAction::EMERGENCY_OFF_REQUESTED:
            logger_.logInfo(
                "Emergency OFF requested");
            break;

        case ControllerAction::INVALID_INPUT_IGNORED:
            logger_.logError(
                "INVALID_INPUT: " + result.detail);
            break;

        case ControllerAction::QUIT_REQUESTED:
            logger_.logInfo(
                "Quit requested");

            running_ = false;
            break;

        default:
            break;
        }

        // =================================================
        // LOG STATE TRANSITION
        // =================================================

        // Không chỉ dựa vào stateChanged.
        // Controller hiện tại khi thoát EMERGENCY có thể đổi state
        // nhưng ControllerResult.stateChanged chưa chắc được set true.
        if (result.previousState != result.currentState)
        {
            logger_.logStateTransition(
                result.previousState,
                result.currentState);

            // Vào PED_WALK nghĩa là pedestrian request
            // bắt đầu được phục vụ.
            if (result.currentState ==
                TrafficState::PED_WALK)
            {
                logger_.logPedestrianServed();
            }

            // Chỉ log "Emergency mode ON"
            // khi FSM thực sự đã vào EMERGENCY.
            if (result.currentState ==
                    TrafficState::EMERGENCY &&
                result.previousState !=
                    TrafficState::EMERGENCY)
            {
                logger_.logEmergency(true);
            }

            // Chỉ log OFF khi FSM thực sự rời EMERGENCY.
            if (result.previousState ==
                    TrafficState::EMERGENCY &&
                result.currentState !=
                    TrafficState::EMERGENCY)
            {
                logger_.logEmergency(false);
            }
        }
    }

    void Application::render(
        const std::string &command)
    {
        const SystemSnapshot snapshot =
            controller_.snapshot();

        const LightOutput lights =
            output_.fromState(snapshot.state);

        display_.show(
            snapshot,
            lights,
            command);
    }

} // namespace traffic