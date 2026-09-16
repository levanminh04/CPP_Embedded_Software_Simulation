#include "traffic/ConsoleInput.h"
#include "traffic/Controller.h"
#include "traffic/InputValidator.h"
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

int main()
{
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

    assert(controller.state() == TrafficState::STARTUP_ALL_RED);
    assert(controller.remainingSeconds() == cycleConfig.allRedTime);

    advance(controller, cycleConfig.allRedTime);
    assert(controller.state() == TrafficState::NS_GREEN);
    assert(controller.nextDirection() == Direction::EW);
    assert(controller.remainingSeconds() == cycleConfig.greenLowTime);

    advance(controller, cycleConfig.greenLowTime);
    assert(controller.state() == TrafficState::NS_YELLOW);
    assert(controller.remainingSeconds() == cycleConfig.yellowTime);

    advance(controller, cycleConfig.yellowTime);
    assert(controller.state() == TrafficState::ALL_RED);
    assert(controller.remainingSeconds() == cycleConfig.allRedTime);

    advance(controller, cycleConfig.allRedTime);
    assert(controller.state() == TrafficState::EW_GREEN);
    assert(controller.nextDirection() == Direction::NS);
    assert(controller.remainingSeconds() == cycleConfig.greenHighTime);

    const int activeEwGreenRemaining = controller.remainingSeconds();
    controller.updateSensor(Direction::EW, cycleSensor.update(Direction::EW, 0));
    controller.updateSensor(Direction::NS, cycleSensor.update(Direction::NS, 20));
    assert(controller.remainingSeconds() == activeEwGreenRemaining);

    advance(controller, cycleConfig.greenHighTime);
    assert(controller.state() == TrafficState::EW_YELLOW);
    assert(controller.remainingSeconds() == cycleConfig.yellowTime);

    advance(controller, cycleConfig.yellowTime);
    assert(controller.state() == TrafficState::ALL_RED);
    assert(controller.remainingSeconds() == cycleConfig.allRedTime);

    advance(controller, cycleConfig.allRedTime);
    assert(controller.state() == TrafficState::NS_GREEN);
    assert(controller.remainingSeconds() == cycleConfig.greenHighTime);

    Controller pendingRequestsController(cycleConfig);
    pendingRequestsController.apply({EventType::PEDESTRIAN_REQUEST, Direction::NS, 0, ""});
    pendingRequestsController.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    assert(pendingRequestsController.state() == TrafficState::STARTUP_ALL_RED);
    assert(pendingRequestsController.pedestrianRequested());
    assert(pendingRequestsController.emergencyPending());

    cout << "Smoke tests passed.\n";
    return 0;
}
