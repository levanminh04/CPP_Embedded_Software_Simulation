#include "traffic/Controller.h"
#include "traffic/EventSimulator.h"
#include "traffic/InputValidator.h"
#include "traffic/Sensor.h"

#include <cassert>
#include <iostream>

using namespace std;
using namespace traffic;

void advance(Controller& controller, int seconds) {
    for (int i = 0; i < seconds; ++i) {
        controller.tick();
    }
}

int main() {
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

    EventSimulator events;
    assert(events.parseLine("P").type == EventType::PEDESTRIAN_REQUEST);
    assert(events.parseLine("E").type == EventType::EMERGENCY_TOGGLE);
    assert(events.parseLine("NS abc").type == EventType::INVALID_INPUT);

    Event sensorEvent = events.parseLine("EW 4");
    assert(sensorEvent.type == EventType::SENSOR_UPDATE);
    assert(sensorEvent.direction == Direction::EW);
    assert(sensorEvent.vehicleCount == 4);

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
    assert(controller.state() == TrafficState::ALL_RED_TO_EW);
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
    assert(controller.state() == TrafficState::ALL_RED_TO_NS);
    assert(controller.remainingSeconds() == cycleConfig.allRedTime);

    advance(controller, cycleConfig.allRedTime);
    assert(controller.state() == TrafficState::NS_GREEN);
    assert(controller.remainingSeconds() == cycleConfig.greenHighTime);

    Controller ignoredEventsController(cycleConfig);
    ignoredEventsController.apply({EventType::PEDESTRIAN_REQUEST, Direction::NS, 0, ""});
    ignoredEventsController.apply({EventType::EMERGENCY_TOGGLE, Direction::NS, 0, ""});
    assert(ignoredEventsController.state() == TrafficState::STARTUP_ALL_RED);
    assert(!ignoredEventsController.pedestrianRequested());
    assert(!ignoredEventsController.emergencyPending());

    cout << "Smoke tests passed.\n";
    return 0;
}
