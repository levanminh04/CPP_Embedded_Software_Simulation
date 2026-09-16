#include <iostream>
#include <thread>
#include <chrono>

#include "../include/Timer.h"
#include "../include/Logger.h"

int main()
{
    Timer timer(1000);

    Logger logger("logs/test_timer_logger.log");

    if (!logger.isOpen())
    {
        std::cerr << "ERROR: Cannot open test log file\n";
        return 1;
    }

    std::cout << "===== TEST TIMER + LOGGER =====\n";

    logger.logStatus(
        "GREEN",
        "RED",
        5,
        false);

    logger.logStateTransition(
        "RED",
        "GREEN");

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

            if (tickCount == 2)
            {
                logger.logPedestrianRequest();

                logger.logStatus(
                    "GREEN",
                    "RED",
                    remainingTime,
                    true);
            }

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

        std::this_thread::sleep_for(
            std::chrono::milliseconds(10));
    }

    logger.logEmergency(true);
    logger.logEmergency(false);

    logger.logError(
        "Test invalid sensor value");

    std::cout << "\nTest finished.\n";
    std::cout << "Check: logs/test_timer_logger.log\n";

    return 0;
}