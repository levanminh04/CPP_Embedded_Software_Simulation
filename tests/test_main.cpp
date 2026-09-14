#include "traffic/EventSimulator.h"
#include "traffic/InputValidator.h"
#include "traffic/Sensor.h"

#include <cassert>
#include <iostream>

using namespace std;
using namespace traffic;

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

    cout << "Input sensor tests passed.\n";
    return 0;
}
