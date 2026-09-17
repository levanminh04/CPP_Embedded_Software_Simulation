#include "traffic/ConsoleInput.h"
#include "traffic/Controller.h"
#include "traffic/InputValidator.h"
#include "traffic/Output.h"
#include "traffic/Sensor.h"

#include <cassert>
#include <iostream>

using namespace std;
using namespace traffic;

void advance(Controller &controller, int seconds)
{
    for (int i = 0; i < seconds; ++i)
    {
        controller.tick();
    }
}

Config testConfig()
{
    Config config = Config::defaults();
    config.greenLowTime = 2;
    config.greenMediumTime = 3;
    config.greenHighTime = 4;
    config.yellowTime = 1;
    config.allRedTime = 1;
    config.pedestrianWalkTime = 2;
    config.pedestrianWarningTime = 1;
    assert(config.isValid());
    return config;
}

void testT1NormalTrafficSequence()
{
    // T1: Chế độ bình thường phải đi theo RED -> GREEN -> YELLOW -> RED.
    Controller controller(testConfig());

    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::NS_GREEN);

    advance(controller, 2);
    assert(controller.snapshot().state == TrafficState::NS_YELLOW);

    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::ALL_RED);
}

void testT2PedestrianRequestDuringGreen()
{
    // T2: Nhấn P khi GREEN phải ghi nhận request, sau đó chờ đến điểm an toàn.
    Controller controller(testConfig());
    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::NS_GREEN);

    const ControllerResult requestResult =
        controller.apply({EventType::PEDESTRIAN_REQUEST, Direction::NS, 0, ""});
    assert(requestResult.action == ControllerAction::PEDESTRIAN_REQUEST_RECORDED);
    assert(controller.snapshot().pedestrianRequested);

    advance(controller, 2);
    assert(controller.snapshot().state == TrafficState::NS_YELLOW);
    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::ALL_RED);
    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::PED_WALK);
}

void testT3PedestrianGreenForcesVehicleRed()
{
    // T3: Khi pedestrian GREEN, cả hai hướng xe bắt buộc phải RED.
    const LightOutput lights = Output().fromState(TrafficState::PED_WALK);

    assert(lights.pedestrian == PedestrianLight::GREEN);
    assert(lights.ns == VehicleLight::RED);
    assert(lights.ew == VehicleLight::RED);
}

void testT4EmergencyReachesSafeState()
{
    // T4: Emergency không cắt ngang GREEN; hệ thống phải qua YELLOW và ALL_RED.
    Controller controller(testConfig());
    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::NS_GREEN);

    const ControllerResult emergencyResult =
        controller.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    assert(emergencyResult.action == ControllerAction::EMERGENCY_ON_REQUESTED);
    assert(controller.snapshot().emergencyPending);

    advance(controller, 2);
    assert(controller.snapshot().state == TrafficState::NS_YELLOW);
    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::ALL_RED);
    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::EMERGENCY);
}

void testT5EmergencyExitReturnsToNormalMode()
{
    // T5: Thoát Emergency đưa hệ thống về ALL_RED rồi trở lại normal mode.
    Controller controller(testConfig());
    controller.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::EMERGENCY);

    const ControllerResult exitResult =
        controller.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    assert(exitResult.action == ControllerAction::EMERGENCY_OFF_REQUESTED);
    assert(controller.snapshot().state == TrafficState::ALL_RED);
    assert(controller.snapshot().nextDirection == Direction::NS);

    advance(controller, 1);
    assert(controller.snapshot().state == TrafficState::NS_GREEN);
}

int main()
{
    testT1NormalTrafficSequence();
    testT2PedestrianRequestDuringGreen();
    testT3PedestrianGreenForcesVehicleRed();
    testT4EmergencyReachesSafeState();
    testT5EmergencyExitReturnsToNormalMode();

    Config config = Config::defaults();
    assert(config.isValid());

    TrafficDensitySensor sensor(config);
    assert(sensor.update(Direction::NS, 3).density == TrafficDensity::LOW);
    assert(sensor.update(Direction::NS, 10).density == TrafficDensity::MEDIUM);
    assert(sensor.update(Direction::NS, 20).density == TrafficDensity::HIGH);

    SensorReading invalidReading = sensor.update(Direction::EW, -5);
    assert(!invalidReading.valid);
    assert(invalidReading.density == TrafficDensity::LOW);

    int vehicleCount;
    assert(!parseVehicleCount("abc", vehicleCount));
    assert(parseVehicleCount("20", vehicleCount));
    assert(vehicleCount == 20);

    ConsoleInput input;
    Event event;

    // Ký tự đơn chưa có Enter -> chưa trả event
    assert(!input.feedKey('E', event));
    assert(input.currentBuffer() == "E");

    // Nhập tiếp 'W', ' ', '4' -> vẫn chưa có event
    assert(!input.feedKey('W', event));
    assert(!input.feedKey(' ', event));
    assert(!input.feedKey('4', event));
    assert(input.currentBuffer() == "EW 4");

    // Enter -> parse xong trả SENSOR_UPDATE và buffer được xóa
    assert(input.feedKey('\n', event));
    assert(event.type == EventType::SENSOR_UPDATE);
    assert(event.direction == Direction::EW);
    assert(event.vehicleCount == 4);
    assert(input.currentBuffer().empty());

    // Nhập riêng 'E' rồi Enter -> EMERGENCY_TOGGLE (không bị nhầm với EW)
    assert(!input.feedKey('E', event));
    assert(input.feedKey('\n', event));
    assert(event.type == EventType::EMERGENCY_TOGGLE);

    // Phím P -> PEDESTRIAN_REQUEST
    assert(!input.feedKey('P', event));
    assert(input.feedKey('\n', event));
    assert(event.type == EventType::PEDESTRIAN_REQUEST);

    // Lệnh lỗi cú pháp
    assert(!input.feedKey('N', event));
    assert(!input.feedKey('S', event));
    assert(!input.feedKey(' ', event));
    assert(!input.feedKey('a', event));
    assert(!input.feedKey('b', event));
    assert(!input.feedKey('c', event));
    assert(input.feedKey('\n', event));
    assert(event.type == EventType::INVALID_INPUT);

    Config cycleConfig = Config::defaults();
    cycleConfig.greenLowTime = 2;
    cycleConfig.greenMediumTime = 3;
    cycleConfig.greenHighTime = 4;
    cycleConfig.yellowTime = 1;
    cycleConfig.allRedTime = 1;
    assert(cycleConfig.isValid());

    TrafficDensitySensor cycleSensor(cycleConfig);
    Controller controller(cycleConfig);
    controller.updateSensor(Direction::EW, cycleSensor.update(Direction::EW, 20));

    assert(controller.snapshot().state == TrafficState::STARTUP_ALL_RED);
    assert(controller.snapshot().remainingSeconds == cycleConfig.allRedTime);

    advance(controller, cycleConfig.allRedTime);
    assert(controller.snapshot().state == TrafficState::NS_GREEN);
    assert(controller.snapshot().nextDirection == Direction::EW);
    assert(controller.snapshot().remainingSeconds == cycleConfig.greenLowTime);

    advance(controller, cycleConfig.greenLowTime);
    assert(controller.snapshot().state == TrafficState::NS_YELLOW);
    assert(controller.snapshot().remainingSeconds == cycleConfig.yellowTime);

    advance(controller, cycleConfig.yellowTime);
    assert(controller.snapshot().state == TrafficState::ALL_RED);
    assert(controller.snapshot().remainingSeconds == cycleConfig.allRedTime);

    advance(controller, cycleConfig.allRedTime);
    assert(controller.snapshot().state == TrafficState::EW_GREEN);
    assert(controller.snapshot().nextDirection == Direction::NS);
    assert(controller.snapshot().remainingSeconds == cycleConfig.greenHighTime);

    const int activeEwGreenRemaining = controller.snapshot().remainingSeconds;
    controller.updateSensor(Direction::EW, cycleSensor.update(Direction::EW, 0));
    controller.updateSensor(Direction::NS, cycleSensor.update(Direction::NS, 20));
    assert(controller.snapshot().remainingSeconds == activeEwGreenRemaining);

    advance(controller, cycleConfig.greenHighTime);
    assert(controller.snapshot().state == TrafficState::EW_YELLOW);
    assert(controller.snapshot().remainingSeconds == cycleConfig.yellowTime);

    advance(controller, cycleConfig.yellowTime);
    assert(controller.snapshot().state == TrafficState::ALL_RED);
    assert(controller.snapshot().remainingSeconds == cycleConfig.allRedTime);

    advance(controller, cycleConfig.allRedTime);
    assert(controller.snapshot().state == TrafficState::NS_GREEN);
    assert(controller.snapshot().remainingSeconds == cycleConfig.greenHighTime);

    Controller pendingRequestsController(cycleConfig);
    pendingRequestsController.apply({EventType::PEDESTRIAN_REQUEST, Direction::NS, 0, ""});
    pendingRequestsController.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    assert(pendingRequestsController.snapshot().state == TrafficState::STARTUP_ALL_RED);
    assert(pendingRequestsController.snapshot().pedestrianRequested);
    assert(pendingRequestsController.snapshot().emergencyPending);

    // Pedestrian request should be served only at a safe decision point, not immediately.
    Controller pedestrianController(cycleConfig);
    pedestrianController.updateSensor(Direction::EW, cycleSensor.update(Direction::EW, 20));
    advance(pedestrianController, cycleConfig.allRedTime);
    assert(pedestrianController.snapshot().state == TrafficState::NS_GREEN);
    pedestrianController.apply({EventType::PEDESTRIAN_REQUEST, Direction::NS, 0, ""});
    assert(pedestrianController.snapshot().pedestrianRequested);

    advance(pedestrianController, cycleConfig.greenLowTime);
    assert(pedestrianController.snapshot().state == TrafficState::NS_YELLOW);
    advance(pedestrianController, cycleConfig.yellowTime);
    assert(pedestrianController.snapshot().state == TrafficState::ALL_RED);
    advance(pedestrianController, cycleConfig.allRedTime);
    assert(pedestrianController.snapshot().state == TrafficState::PED_WALK);
    assert(!pedestrianController.snapshot().pedestrianRequested);
    assert(pedestrianController.snapshot().remainingSeconds == cycleConfig.pedestrianWalkTime);

    // A new request during WALK must wait until after at least one vehicle GREEN cycle.
    Controller deferredPedestrianController(cycleConfig);
    deferredPedestrianController.updateSensor(Direction::EW, cycleSensor.update(Direction::EW, 20));
    advance(deferredPedestrianController, cycleConfig.allRedTime);
    assert(deferredPedestrianController.snapshot().state == TrafficState::NS_GREEN);
    advance(deferredPedestrianController, cycleConfig.greenLowTime);
    advance(deferredPedestrianController, cycleConfig.yellowTime);
    assert(deferredPedestrianController.snapshot().state == TrafficState::ALL_RED);
    deferredPedestrianController.apply({EventType::PEDESTRIAN_REQUEST, Direction::NS, 0, ""});
    advance(deferredPedestrianController, cycleConfig.allRedTime);
    assert(deferredPedestrianController.snapshot().state == TrafficState::PED_WALK);
    deferredPedestrianController.apply({EventType::PEDESTRIAN_REQUEST, Direction::NS, 0, ""});
    advance(deferredPedestrianController, cycleConfig.pedestrianWalkTime);
    assert(deferredPedestrianController.snapshot().state == TrafficState::PED_WARNING);
    advance(deferredPedestrianController, cycleConfig.pedestrianWarningTime);
    assert(deferredPedestrianController.snapshot().state == TrafficState::ALL_RED);
    assert(deferredPedestrianController.snapshot().pedestrianRequested);
    advance(deferredPedestrianController, cycleConfig.allRedTime);
    assert(deferredPedestrianController.snapshot().state == TrafficState::EW_GREEN);

    // Emergency has highest priority and must pass through clearances safely.
    Controller emergencyGreenController(cycleConfig);
    emergencyGreenController.updateSensor(Direction::EW, cycleSensor.update(Direction::EW, 20));
    advance(emergencyGreenController, cycleConfig.allRedTime);
    assert(emergencyGreenController.snapshot().state == TrafficState::NS_GREEN);
    emergencyGreenController.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    assert(emergencyGreenController.snapshot().emergencyPending);
    advance(emergencyGreenController, cycleConfig.greenLowTime);
    assert(emergencyGreenController.snapshot().state == TrafficState::NS_YELLOW);
    advance(emergencyGreenController, cycleConfig.yellowTime);
    assert(emergencyGreenController.snapshot().state == TrafficState::ALL_RED);
    advance(emergencyGreenController, cycleConfig.allRedTime);
    assert(emergencyGreenController.snapshot().state == TrafficState::EMERGENCY);

    Controller emergencyPedestrianController(cycleConfig);
    emergencyPedestrianController.updateSensor(Direction::EW, cycleSensor.update(Direction::EW, 20));
    advance(emergencyPedestrianController, cycleConfig.allRedTime);
    emergencyPedestrianController.apply({EventType::PEDESTRIAN_REQUEST, Direction::NS, 0, ""});
    advance(emergencyPedestrianController, cycleConfig.greenLowTime);
    advance(emergencyPedestrianController, cycleConfig.yellowTime);
    advance(emergencyPedestrianController, cycleConfig.allRedTime);
    assert(emergencyPedestrianController.snapshot().state == TrafficState::PED_WALK);
    emergencyPedestrianController.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    assert(emergencyPedestrianController.snapshot().emergencyPending);
    advance(emergencyPedestrianController, cycleConfig.pedestrianWalkTime);
    assert(emergencyPedestrianController.snapshot().state == TrafficState::PED_WARNING);
    advance(emergencyPedestrianController, cycleConfig.pedestrianWarningTime);
    assert(emergencyPedestrianController.snapshot().state == TrafficState::ALL_RED);
    advance(emergencyPedestrianController, cycleConfig.allRedTime);
    assert(emergencyPedestrianController.snapshot().state == TrafficState::EMERGENCY);

    Controller emergencyExitController(cycleConfig);
    emergencyExitController.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    advance(emergencyExitController, cycleConfig.allRedTime);
    assert(emergencyExitController.snapshot().state == TrafficState::EMERGENCY);
    emergencyExitController.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    assert(!emergencyExitController.snapshot().emergencyPending);
    assert(emergencyExitController.snapshot().state == TrafficState::ALL_RED);
    assert(emergencyExitController.snapshot().nextDirection == Direction::NS);

    cout << "Smoke tests passed.\n";
    return 0;
}