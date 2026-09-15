#include "../include/Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

Logger::Logger(const std::string &filePath)
{
    logFile_.open(filePath, std::ios::app);
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
// Ghi trạng thái giống Console mẫu
// ======================================================

void Logger::logStatus(
    const std::string &vehicleLight,
    const std::string &pedestrianLight,
    int remainingTime,
    bool pedestrianRequested)
{
    if (!logFile_.is_open())
    {
        return;
    }

    logFile_
        << "===== TRAFFIC CONTROLLER =====\n"
        << "Vehicle Light    : "
        << vehicleLight << '\n'

        << "Pedestrian Light : "
        << pedestrianLight << '\n'

        << "Remaining Time   : "
        << remainingTime << " s\n"

        << "Pedestrian Req.  : "
        << (pedestrianRequested ? "YES" : "NO")
        << '\n'

        << "==============================\n"

        << "P = Pedestrian request | E = Emergency\n\n";

    logFile_.flush();
}

// ======================================================
// State transition
// ======================================================

void Logger::logStateTransition(
    const std::string &oldState,
    const std::string &newState)
{
    logEvent(
        "STATE",
        oldState + " -> " + newState);
}

// ======================================================
// Pedestrian request
// ======================================================

void Logger::logPedestrianRequest()
{
    logEvent(
        "EVENT",
        "Pedestrian request received");
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
// Error
// ======================================================

void Logger::logError(const std::string &message)
{
    logEvent(
        "ERROR",
        message);
}

// ======================================================
// Ghi event chung
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
// Lấy thời gian hiện tại
// ======================================================

std::string Logger::getTimestamp() const
{
    auto now =
        std::chrono::system_clock::now();

    std::time_t nowTime =
        std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &nowTime);
#else
    localtime_r(&nowTime, &localTime);
#endif

    std::ostringstream oss;

    oss << std::put_time(
        &localTime,
        "%Y-%m-%d %H:%M:%S");

    return oss.str();
}