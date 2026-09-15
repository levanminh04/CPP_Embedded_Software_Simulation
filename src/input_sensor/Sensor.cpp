#include "traffic/Sensor.h"

namespace traffic {

TrafficDensitySensor::TrafficDensitySensor(Config config)
    : config(config), nsReading(classify(0)), ewReading(classify(0)) {}

SensorReading TrafficDensitySensor::classify(int vehicleCount) {
    if (vehicleCount < 0) {
        return {vehicleCount, TrafficDensity::LOW, false};
    }

    if (vehicleCount <= config.densityLowMax) {
        return {vehicleCount, TrafficDensity::LOW, true};
    }

    if (vehicleCount <= config.densityMediumMax) {
        return {vehicleCount, TrafficDensity::MEDIUM, true};
    }

    return {vehicleCount, TrafficDensity::HIGH, true};
}

SensorReading TrafficDensitySensor::update(Direction direction, int vehicleCount) {
    SensorReading reading = classify(vehicleCount);

    if (direction == Direction::NS) {
        nsReading = reading;
    } else {
        ewReading = reading;
    }

    return reading;
}

SensorReading TrafficDensitySensor::read(Direction direction) {
    if (direction == Direction::NS) {
        return nsReading;
    }

    return ewReading;
}

} 
