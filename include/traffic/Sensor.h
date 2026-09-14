#pragma once

#include "traffic/Config.h"
#include "traffic/Types.h"

namespace traffic {

class TrafficDensitySensor {
private:
    Config config;
    SensorReading nsReading;
    SensorReading ewReading;

    SensorReading classify(int vehicleCount);

public:
    TrafficDensitySensor(Config config = Config::defaults());

    SensorReading update(Direction direction, int vehicleCount);
    SensorReading read(Direction direction);
};

} 
