#include "traffic/Controller.h"

namespace traffic
{

    Controller::Controller(Config config)
        : config(config),
          currentSnapshot(),
          nsReading(),
          ewReading(),
          quitRequestedFlag(false)
    {
        if (!this->config.isValid())
        {
            this->config = Config::defaults();
        }

        nsReading = {0, TrafficDensity::LOW, true};
        ewReading = {0, TrafficDensity::LOW, true};

        currentSnapshot.state = TrafficState::STARTUP_ALL_RED;
        currentSnapshot.nextDirection = Direction::NS;
        currentSnapshot.remainingSeconds = this->config.allRedTime;
        currentSnapshot.nsVehicleCount = nsReading.vehicleCount;
        currentSnapshot.ewVehicleCount = ewReading.vehicleCount;
        currentSnapshot.pedestrianRequested = false;
        currentSnapshot.emergencyPending = false;
    }

    ControllerResult Controller::apply(const Event &event)
    {
        if (event.type == EventType::QUIT)
        {
            requestQuit();
            return {ControllerAction::QUIT_REQUESTED, currentSnapshot.state, currentSnapshot.state,
                    false, event.detail};
        }

        if (event.type == EventType::INVALID_INPUT)
        {
            return {ControllerAction::INVALID_INPUT_IGNORED, currentSnapshot.state,
                    currentSnapshot.state, false, event.detail};
        }

        if (event.type == EventType::PEDESTRIAN_REQUEST)
        {
            requestPedestrian();
            return {ControllerAction::PEDESTRIAN_REQUEST_RECORDED, currentSnapshot.state,
                    currentSnapshot.state, false, "pedestrian request recorded"};
        }

        if (event.type == EventType::EMERGENCY_TOGGLE)
        {
            const TrafficState beforeState = currentSnapshot.state;
            requestEmergencyToggle();

            if (beforeState == TrafficState::EMERGENCY)
            {
                return {ControllerAction::EMERGENCY_OFF_REQUESTED, beforeState,
                        currentSnapshot.state, false, "emergency exit requested"};
            }

            return {ControllerAction::EMERGENCY_ON_REQUESTED, beforeState,
                    currentSnapshot.state, false, "emergency toggle requested"};
        }

        if (event.type == EventType::SENSOR_UPDATE)
        {
            return updateSensor(event.direction, {event.vehicleCount,
                                                  event.vehicleCount <= config.densityLowMax      ? TrafficDensity::LOW
                                                  : event.vehicleCount <= config.densityMediumMax ? TrafficDensity::MEDIUM
                                                                                                  : TrafficDensity::HIGH,
                                                  event.vehicleCount >= 0});
        }

        return makeNoChangeResult();
    }

    ControllerResult Controller::tick()
    {
        const TrafficState previousState = currentSnapshot.state;

        ++currentSnapshot.simulationSecond;
        decrementTimer();

        if (currentSnapshot.remainingSeconds == 0)
        {
            handleExpiredState();
        }

        if (currentSnapshot.state != previousState)
        {
            return makeStateChangeResult(previousState, currentSnapshot.state);
        }

        return makeNoChangeResult();
    }

    ControllerResult Controller::updateSensor(Direction direction, const SensorReading &reading)
    {
        if (!reading.valid)
        {
            return {ControllerAction::SENSOR_ERROR_RECORDED, currentSnapshot.state,
                    currentSnapshot.state, false, "invalid sensor reading"};
        }

        storeSensorReading(direction, reading);
        return {ControllerAction::SENSOR_UPDATE_RECORDED, currentSnapshot.state,
                currentSnapshot.state, false, ""};
    }

    SystemSnapshot Controller::snapshot() const
    {
        return currentSnapshot;
    }

    TrafficState Controller::state() const
    {
        return currentSnapshot.state;
    }

    Direction Controller::nextDirection() const
    {
        return currentSnapshot.nextDirection;
    }

    int Controller::remainingSeconds() const
    {
        return currentSnapshot.remainingSeconds;
    }

    bool Controller::pedestrianRequested() const
    {
        return currentSnapshot.pedestrianRequested;
    }

    bool Controller::emergencyPending() const
    {
        return currentSnapshot.emergencyPending;
    }

    bool Controller::quitRequested() const
    {
        return quitRequestedFlag;
    }

    ControllerResult Controller::makeNoChangeResult() const
    {
        return {ControllerAction::NONE, currentSnapshot.state, currentSnapshot.state, false, ""};
    }

    ControllerResult Controller::makeStateChangeResult(TrafficState previousState,
                                                       TrafficState nextState) const
    {
        return {ControllerAction::STATE_CHANGE, previousState, nextState, true, ""};
    }

    void Controller::requestPedestrian()
    {
        // Pedestrian request flow:
        // 1. P được ghi nhận như một cờ chờ (pending) chứ không tạo hàng đợi.
        // 2. Khi đang chờ, controller KHÔNG bắt đầu WALK ngay trong pha xe đang chạy.
        // 3. Yêu cầu chỉ được phục vụ ở điểm an toàn gần nhất:
        //    - xe hiện tại đã hết GREEN
        //    - đã qua YELLOW
        //    - đã có ALL_RED / clearance an toàn
        // 4. Nếu request mới xuất hiện trong PED_WALK, nó sẽ chờ tới lượt xe kế tiếp.
        if (!currentSnapshot.pedestrianRequested)
        {
            currentSnapshot.pedestrianRequested = true;
        }
    }

    void Controller::requestEmergencyToggle()
    {
        // Emergency priority:
        // - E lần 1: đặt cờ khẩn cấp, nhưng không cắt bừa khi đang ở clearance.
        // - Nếu đã ở EMERGENCY, E lần 2 yêu cầu thoát ra khỏi chế độ khẩn cấp.
        // - Trong YELLOW / ALL_RED / PED_WALK / PED_WARNING, E lặp lại không reset timer.
        if (currentSnapshot.state == TrafficState::EMERGENCY)
        {
            currentSnapshot.emergencyPending = false;
            currentSnapshot.state = TrafficState::ALL_RED;
            currentSnapshot.remainingSeconds = config.allRedTime;
            currentSnapshot.nextDirection = Direction::NS;
            return;
        }

        if (currentSnapshot.emergencyPending)
        {
            return;
        }

        if (currentSnapshot.state == TrafficState::NS_YELLOW ||
            currentSnapshot.state == TrafficState::EW_YELLOW ||
            currentSnapshot.state == TrafficState::ALL_RED ||
            currentSnapshot.state == TrafficState::PED_WALK ||
            currentSnapshot.state == TrafficState::PED_WARNING)
        {
            currentSnapshot.emergencyPending = true;
            return;
        }

        currentSnapshot.emergencyPending = true;
    }

    void Controller::requestQuit()
    {
        quitRequestedFlag = true;
    }

    void Controller::storeSensorReading(Direction direction, const SensorReading &reading)
    {
        if (direction == Direction::NS)
        {
            nsReading = reading;
            currentSnapshot.nsVehicleCount = reading.vehicleCount;
            return;
        }

        ewReading = reading;
        currentSnapshot.ewVehicleCount = reading.vehicleCount;
    }

    void Controller::enterState(TrafficState nextState, int durationSeconds)
    {
        currentSnapshot.state = nextState;
        currentSnapshot.remainingSeconds = durationSeconds;

        if (nextState == TrafficState::NS_GREEN || nextState == TrafficState::EW_GREEN)
        {
            pedestrianCooldown = false;
        }
    }

    void Controller::decrementTimer()
    {
        if (currentSnapshot.remainingSeconds > 0)
        {
            --currentSnapshot.remainingSeconds;
        }
    }

    void Controller::handleExpiredState()
    {
        // Pedestrian decision point:
        // - Sau khi xe GREEN hết thời gian, controller chuyển sang YELLOW.
        // - Sau YELLOW, controller đi vào ALL_RED để đảm bảo clearance an toàn.
        // - Sau đó mới xét: Emergency? Pedestrian request? Lượt xe kế tiếp?
        // - Theo requirement, pedestrian request không được xóa khi bắt đầu service,
        //   mà phải giữ cho tới khi WALK bắt đầu, rồi mới mark là đã phục vụ.
        switch (currentSnapshot.state)
        {
        case TrafficState::STARTUP_ALL_RED:
        case TrafficState::ALL_RED:
            if (currentSnapshot.emergencyPending)
            {
                currentSnapshot.state = TrafficState::EMERGENCY;
                currentSnapshot.remainingSeconds = 0;
                break;
            }
            if (shouldServePedestrian())
            {
                startPedestrianPhase();
            }
            else
            {
                chooseNextVehiclePhase();
            }
            break;
        case TrafficState::NS_GREEN:
            if (currentSnapshot.emergencyPending)
            {
                enterState(TrafficState::NS_YELLOW, config.yellowTime);
                break;
            }
            enterState(TrafficState::NS_YELLOW, config.yellowTime);
            break;
        case TrafficState::EW_GREEN:
            if (currentSnapshot.emergencyPending)
            {
                enterState(TrafficState::EW_YELLOW, config.yellowTime);
                break;
            }
            enterState(TrafficState::EW_YELLOW, config.yellowTime);
            break;
        case TrafficState::NS_YELLOW:
        case TrafficState::EW_YELLOW:
            if (currentSnapshot.emergencyPending)
            {
                enterState(TrafficState::ALL_RED, config.allRedTime);
                break;
            }
            enterState(TrafficState::ALL_RED, config.allRedTime);
            break;
        case TrafficState::PED_WALK:
            if (currentSnapshot.emergencyPending)
            {
                enterState(TrafficState::PED_WARNING, config.pedestrianWarningTime);
                break;
            }
            enterState(TrafficState::PED_WARNING, config.pedestrianWarningTime);
            break;
        case TrafficState::PED_WARNING:
            if (currentSnapshot.emergencyPending)
            {
                enterState(TrafficState::ALL_RED, config.allRedTime);
                break;
            }
            enterState(TrafficState::ALL_RED, config.allRedTime);
            break;
        case TrafficState::EMERGENCY:
            break;
        }
    }

    void Controller::chooseNextVehiclePhase()
    {
        const Direction activeDirection = currentSnapshot.nextDirection;
        const TrafficDensity sampledDensity = readTrafficDensity(activeDirection);
        const TrafficState nextGreenState = greenStateFor(activeDirection);

        // Density is sampled exactly before a vehicle GREEN starts.
        // The selected duration is fixed for this GREEN turn.
        // Sensor updates during the turn are kept for the next turn.
        enterState(nextGreenState, greenDurationFor(sampledDensity));
        currentSnapshot.nextDirection = opposite(activeDirection);
    }

    void Controller::chooseAllRedDestination()
    {
        enterState(TrafficState::ALL_RED, config.allRedTime);
    }

    void Controller::startPedestrianPhase()
    {
        // Safe pedestrian service start:
        // - request đang chờ là điều kiện bắt buộc
        // - khi bắt đầu WALK thì request được xóa, vì đã được chấp nhận và phục vụ
        // - ALL_RED và YELLOW của xe đã kết thúc, nên người đi bộ mới được phép đi
        if (!currentSnapshot.pedestrianRequested)
        {
            return;
        }

        currentSnapshot.pedestrianRequested = false;
        pedestrianCooldown = true;
        enterState(TrafficState::PED_WALK, config.pedestrianWalkTime);
    }

    void Controller::startEmergencyClearance() {}

    int Controller::durationFor(TrafficState state, Direction direction) const
    {
        if (state == TrafficState::NS_GREEN || state == TrafficState::EW_GREEN)
        {
            return greenDurationFor(readTrafficDensity(direction));
        }

        if (state == TrafficState::NS_YELLOW || state == TrafficState::EW_YELLOW)
        {
            return config.yellowTime;
        }

        if (isAllRed(state))
        {
            return config.allRedTime;
        }

        if (state == TrafficState::PED_WALK)
        {
            return config.pedestrianWalkTime;
        }

        if (state == TrafficState::PED_WARNING)
        {
            return config.pedestrianWarningTime;
        }

        return 0;
    }

    TrafficDensity Controller::readTrafficDensity(Direction direction) const
    {
        const SensorReading &reading = direction == Direction::NS ? nsReading : ewReading;
        return reading.density;
    }

    int Controller::greenDurationFor(TrafficDensity density) const
    {
        if (density == TrafficDensity::HIGH)
        {
            return config.greenHighTime;
        }

        if (density == TrafficDensity::MEDIUM)
        {
            return config.greenMediumTime;
        }

        return config.greenLowTime;
    }

    TrafficState Controller::greenStateFor(Direction direction) const
    {
        return direction == Direction::NS ? TrafficState::NS_GREEN : TrafficState::EW_GREEN;
    }

    TrafficState Controller::yellowStateFor(Direction direction) const
    {
        return direction == Direction::NS ? TrafficState::NS_YELLOW : TrafficState::EW_YELLOW;
    }

    TrafficState Controller::allRedStateToward(Direction direction) const
    {
        (void)direction;
        return TrafficState::ALL_RED;
    }

    Direction Controller::opposite(Direction direction) const
    {
        return direction == Direction::NS ? Direction::EW : Direction::NS;
    }

    bool Controller::isVehicleGreen(TrafficState state) const
    {
        return state == TrafficState::NS_GREEN || state == TrafficState::EW_GREEN;
    }

    bool Controller::isVehicleYellow(TrafficState state) const
    {
        return state == TrafficState::NS_YELLOW || state == TrafficState::EW_YELLOW;
    }

    bool Controller::isAllRed(TrafficState state) const
    {
        return state == TrafficState::STARTUP_ALL_RED || state == TrafficState::ALL_RED;
    }

    bool Controller::isPedestrianPhase(TrafficState state) const
    {
        return state == TrafficState::PED_WALK || state == TrafficState::PED_WARNING;
    }

    bool Controller::isEmergencyState(TrafficState state) const
    {
        return state == TrafficState::EMERGENCY;
    }

    bool Controller::shouldServePedestrian() const
    {
        // Pedestrian safety rule:
        // - chỉ phục vụ khi đã qua vòng xe hiện tại và đang ở trạng thái an toàn ALL_RED
        // - không phục vụ khi Emergency đang active
        // - yêu cầu phải còn đang chờ
        // - không phục vụ lặp lại ngay trong PED_WALK/PED_WARNING
        if (currentSnapshot.emergencyPending || currentSnapshot.state == TrafficState::EMERGENCY)
        {
            return false;
        }

        if (pedestrianCooldown)
        {
            return false;
        }

        if (!currentSnapshot.pedestrianRequested)
        {
            return false;
        }

        if (currentSnapshot.state == TrafficState::PED_WALK ||
            currentSnapshot.state == TrafficState::PED_WARNING)
        {
            return false;
        }

        return currentSnapshot.state == TrafficState::ALL_RED ||
               currentSnapshot.state == TrafficState::STARTUP_ALL_RED;
    }

}
