#include <iostream>
#include <thread>
#include <chrono>

#include "Timer.h"
#include "Logger.h"

int main()
{
    // 1 tick = 1000 ms = 1 giây
    Timer timer(1000);

    // Ghi log vào file logs/system.log
    Logger logger("logs/system.log");

    if (!logger.isOpen())
    {
        std::cerr << "ERROR: Cannot open logs/system.log\n";
        return 1;
    }

    std::cout << "===== TEST TIMER + LOGGER =====\n";

    // Test Logger - trạng thái ban đầu
    logger.logStatus(
        "GREEN",
        "RED",
        5,
        false);

    logger.logStateTransition(
        "RED",
        "GREEN");

    std::cout << "Timer test started...\n";
    std::cout << "Expected: 1 tick / second\n\n";

    int tickCount = 0;
    int remainingTime = 5;

    while (tickCount < 5)
    {

        if (timer.isTick())
        {

            ++tickCount;
            --remainingTime;

            std::cout
                << "Tick "
                << tickCount
                << " | Remaining: "
                << remainingTime
                << " s\n";

            // Mô phỏng người đi bộ request ở tick thứ 2
            if (tickCount == 2)
            {
                logger.logPedestrianRequest();

                logger.logStatus(
                    "GREEN",
                    "RED",
                    remainingTime,
                    true);
            }

            // Mô phỏng GREEN -> YELLOW
            if (tickCount == 4)
            {
                logger.logStateTransition(
                    "GREEN",
                    "YELLOW");

                logger.logStatus(
                    "YELLOW",
                    "RED",
                    3,
                    true);
            }
        }

        // Tránh vòng while chạy 100% CPU
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10));
    }

    // Test Emergency
    logger.logEmergency(true);

    std::this_thread::sleep_for(
        std::chrono::milliseconds(500));

    logger.logEmergency(false);

    // Test Error
    logger.logError(
        "Test invalid sensor value");

    std::cout << "\nTest finished.\n";
    std::cout << "Check file: logs/system.log\n";

    return 0;
}