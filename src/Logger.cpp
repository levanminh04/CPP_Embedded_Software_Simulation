#include "../include/traffic/Logger.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>

Logger::Logger(const std::string &filePath)
{
    const std::filesystem::path path(filePath);

    if (path.has_parent_path())
    {
        std::filesystem::create_directories(
            path.parent_path());
    }

    logFile_.open(
        filePath,
        std::ios::app);
}

Logger::~Logger()
{
    if (logFile_.is_open())
    {
        logFile_.close();
    }
}

bool Logger::isOpen() const
{
    return logFile_.is_open();
}

// ======================================================
// State transition
// ======================================================

void Logger::logStateTransition(
    traffic::TrafficState oldState,
    traffic::TrafficState newState)
{
    logEvent(
        "STATE",
        stateToString(oldState) + " -> " + stateToString(newState));
}

// ======================================================
// Pedestrian request
// ======================================================

void Logger::logPedestrianRequest()
{
    logEvent(
        "PEDESTRIAN",
        "Pedestrian request received");
}

// ======================================================
// Pedestrian served
// ======================================================

void Logger::logPedestrianServed()
{
    logEvent(
        "PEDESTRIAN",
        "Pedestrian request served");
}

// ======================================================
// Emergency
// ======================================================

void Logger::logEmergency(bool enabled)
{
    if (enabled)
    {
        logEvent(
            "EMERGENCY",
            "Emergency mode ON");
    }
    else
    {
        logEvent(
            "EMERGENCY",
            "Emergency mode OFF");
    }
}

// ======================================================
// Sensor reading
// ======================================================

void Logger::logSensorReading(
    traffic::Direction direction,
    int vehicleCount)
{
    logEvent(
        "SENSOR",
        directionToString(direction) + " vehicle count = " + std::to_string(vehicleCount));
}

// ======================================================
// Error
// ======================================================

void Logger::logError(
    const std::string &message)
{
    logEvent(
        "ERROR",
        message);
}

// ======================================================
// Info
// ======================================================

void Logger::logInfo(
    const std::string &message)
{
    logEvent(
        "INFO",
        message);
}

// ======================================================
// TrafficState -> string
// ======================================================

std::string Logger::stateToString(
    traffic::TrafficState state) const
{
    switch (state)
    {
    case traffic::TrafficState::STARTUP_ALL_RED:
        return "STARTUP_ALL_RED";

    case traffic::TrafficState::NS_GREEN:
        return "NS_GREEN";

    case traffic::TrafficState::NS_YELLOW:
        return "NS_YELLOW";

    case traffic::TrafficState::ALL_RED:
        return "ALL_RED";

    case traffic::TrafficState::EW_GREEN:
        return "EW_GREEN";

    case traffic::TrafficState::EW_YELLOW:
        return "EW_YELLOW";

    case traffic::TrafficState::PED_WALK:
        return "PED_WALK";

    case traffic::TrafficState::PED_WARNING:
        return "PED_WARNING";

    case traffic::TrafficState::EMERGENCY:
        return "EMERGENCY";
    }

    return "UNKNOWN";
}

// ======================================================
// Direction -> string
// ======================================================

std::string Logger::directionToString(
    traffic::Direction direction) const
{
    switch (direction)
    {
    case traffic::Direction::NS:
        return "NS";

    case traffic::Direction::EW:
        return "EW";
    }

    return "UNKNOWN";
}

// ======================================================
// Common log writer
// ======================================================

void Logger::logEvent(
    const std::string &type,
    const std::string &message)
{
    if (!logFile_.is_open())
    {
        return;
    }

    logFile_
        << "["
        << getTimestamp()
        << "] ["
        << type
        << "] "
        << message
        << '\n';

    logFile_.flush();
}

// ======================================================
// Timestamp
// ======================================================

std::string Logger::getTimestamp() const
{
    const auto now =
        std::chrono::system_clock::now();

    const std::time_t nowTime =
        std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    localtime_s(
        &localTime,
        &nowTime);
#else
    localtime_r(
        &nowTime,
        &localTime);
#endif

    std::ostringstream oss;

    oss << std::put_time(
        &localTime,
        "%Y-%m-%d %H:%M:%S");

    return oss.str();
}