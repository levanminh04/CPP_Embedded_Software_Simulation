#pragma once

#include <string>

namespace traffic {

enum class Direction {
    NS,
    EW
};

enum class TrafficDensity {
    LOW,
    MEDIUM,
    HIGH
};

struct SensorReading {
    int vehicleCount = 0;
    TrafficDensity density = TrafficDensity::LOW;
    bool valid = true;
};

enum class TrafficState {
    STARTUP_ALL_RED,
    NS_GREEN,
    NS_YELLOW,
    ALL_RED,
    EW_GREEN,
    EW_YELLOW,
    PED_WALK,
    PED_WARNING,
    EMERGENCY
};

enum class VehicleLight {
    RED,
    GREEN,
    YELLOW
};

enum class PedestrianLight {
    RED,
    GREEN,
    WARNING
};

enum class EventType {
    PEDESTRIAN_REQUEST,
    EMERGENCY_TOGGLE,
    SENSOR_UPDATE,
    QUIT,
    INVALID_INPUT
};

struct Event {
    EventType type = EventType::INVALID_INPUT;
    Direction direction = Direction::NS;
    int vehicleCount = 0;
    std::string detail;
};

struct LightOutput {
    VehicleLight ns = VehicleLight::RED;
    VehicleLight ew = VehicleLight::RED;
    PedestrianLight pedestrian = PedestrianLight::RED;
};

struct SystemSnapshot {
    long long simulationSecond = 0;
    TrafficState state = TrafficState::STARTUP_ALL_RED;
    Direction nextDirection = Direction::NS;
    int remainingSeconds = 0;
    int nsVehicleCount = 0;
    int ewVehicleCount = 0;
    bool pedestrianRequested = false;
    bool emergencyPending = false;
};

} // namespace traffic
