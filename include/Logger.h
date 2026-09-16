#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

class Logger
{
public:
    explicit Logger(
        const std::string &filePath = "logs/system.log");

    ~Logger();

    bool isOpen() const;

    // Ghi trạng thái hệ thống theo format Console mẫu
    void logStatus(
        const std::string &vehicleLight,
        const std::string &pedestrianLight,
        int remainingTime,
        bool pedestrianRequested);

    // Ghi các event bắt buộc
    void logStateTransition(
        const std::string &oldState,
        const std::string &newState);

    void logPedestrianRequest();

    void logEmergency(bool enabled);

    void logError(const std::string &message);

private:
    std::ofstream logFile_;

    std::string getTimestamp() const;

    void logEvent(
        const std::string &type,
        const std::string &message);
};

#endif