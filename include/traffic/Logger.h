#pragma once

#include <fstream>
#include <string>

#include "Types.h"

class Logger
{
public:
    explicit Logger(
        const std::string &filePath = "logs/system.log");

    ~Logger();

    bool isOpen() const;

    void logStateTransition(
        traffic::TrafficState oldState,
        traffic::TrafficState newState);

    void logPedestrianRequest();

    void logPedestrianServed();

    void logEmergency(bool enabled);

    void logSensorReading(
        traffic::Direction direction,
        int vehicleCount);

    void logError(
        const std::string &message);

    void logInfo(
        const std::string &message);

private:
    std::ofstream logFile_;

    std::string getTimestamp() const;

    std::string stateToString(
        traffic::TrafficState state) const;

    std::string directionToString(
        traffic::Direction direction) const;

    void logEvent(
        const std::string &type,
        const std::string &message);
};