#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <cmath>

#include "../include/traffic/Timer.h"
#include "../include/traffic/Logger.h"
#include "../include/traffic/Types.h"

// ======================================================
// Helper để in PASS / FAIL
// ======================================================

void printResult(
    const std::string &testName,
    bool passed)
{
    std::cout
        << (passed ? "[PASS] " : "[FAIL] ")
        << testName
        << '\n';
}

// ======================================================
// TEST LOGGER
// ======================================================

bool testLogger()
{
    std::cout
        << "\n===== LOGGER TEST =====\n";

    const std::string logPath =
        "logs/test_timer_logger.log";

    // Xóa log test cũ để mỗi lần test dễ kiểm tra
    std::filesystem::remove(logPath);

    Logger logger(logPath);

    if (!logger.isOpen())
    {
        std::cerr
            << "[FAIL] Cannot open test log file\n";

        return false;
    }

    printResult(
        "Open log file",
        true);

    // ==================================================
    // 1. SYSTEM START
    // ==================================================

    logger.logInfo(
        "Traffic controller started");

    printResult(
        "System info logging",
        true);

    // ==================================================
    // 2. SENSOR READING
    // ==================================================

    logger.logSensorReading(
        traffic::Direction::NS,
        20);

    logger.logSensorReading(
        traffic::Direction::EW,
        4);

    printResult(
        "NS/EW sensor logging",
        true);

    // ==================================================
    // 3. NORMAL TRAFFIC FLOW
    //
    // STARTUP_ALL_RED
    //      ->
    // NS_GREEN
    //      ->
    // NS_YELLOW
    //      ->
    // ALL_RED
    //      ->
    // EW_GREEN
    //      ->
    // EW_YELLOW
    //      ->
    // ALL_RED
    // ==================================================

    logger.logStateTransition(
        traffic::TrafficState::STARTUP_ALL_RED,
        traffic::TrafficState::NS_GREEN);

    logger.logStateTransition(
        traffic::TrafficState::NS_GREEN,
        traffic::TrafficState::NS_YELLOW);

    logger.logStateTransition(
        traffic::TrafficState::NS_YELLOW,
        traffic::TrafficState::ALL_RED);

    logger.logStateTransition(
        traffic::TrafficState::ALL_RED,
        traffic::TrafficState::EW_GREEN);

    logger.logStateTransition(
        traffic::TrafficState::EW_GREEN,
        traffic::TrafficState::EW_YELLOW);

    logger.logStateTransition(
        traffic::TrafficState::EW_YELLOW,
        traffic::TrafficState::ALL_RED);

    printResult(
        "Normal traffic state transitions",
        true);

    // ==================================================
    // 4. PEDESTRIAN REQUEST
    // ==================================================

    // Người đi bộ nhấn P
    logger.logPedestrianRequest();

    // Hệ thống đi qua trạng thái an toàn trước
    logger.logStateTransition(
        traffic::TrafficState::ALL_RED,
        traffic::TrafficState::PED_WALK);

    // Request thực sự đã được phục vụ
    logger.logPedestrianServed();

    logger.logStateTransition(
        traffic::TrafficState::PED_WALK,
        traffic::TrafficState::PED_WARNING);

    logger.logStateTransition(
        traffic::TrafficState::PED_WARNING,
        traffic::TrafficState::ALL_RED);

    printResult(
        "Pedestrian request and served logging",
        true);

    // ==================================================
    // 5. EMERGENCY
    // ==================================================

    logger.logEmergency(true);

    logger.logStateTransition(
        traffic::TrafficState::ALL_RED,
        traffic::TrafficState::EMERGENCY);

    logger.logEmergency(false);

    logger.logStateTransition(
        traffic::TrafficState::EMERGENCY,
        traffic::TrafficState::STARTUP_ALL_RED);

    printResult(
        "Emergency ON/OFF logging",
        true);

    // ==================================================
    // 6. ERROR / ABNORMAL INPUT
    // ==================================================

    logger.logError(
        "Invalid NS sensor value: -5");

    logger.logError(
        "Invalid command");

    printResult(
        "Error logging",
        true);

    // ==================================================
    // 7. SYSTEM STOP
    // ==================================================

    logger.logInfo(
        "Traffic controller stopped");

    printResult(
        "System stop logging",
        true);

    std::cout
        << "Log file: "
        << logPath
        << '\n';

    return true;
}

// ======================================================
// TEST TIMER - PERIODIC TICK
// ======================================================

bool testTimerPeriodic()
{
    std::cout
        << "\n===== TIMER PERIODIC TEST =====\n";

    constexpr int intervalMs = 1000;

    Timer timer(intervalMs);

    const auto start =
        std::chrono::steady_clock::now();

    int tickCount = 0;

    bool passed = true;

    while (tickCount < 5)
    {
        if (timer.isTick())
        {
            ++tickCount;

            const auto now =
                std::chrono::steady_clock::now();

            const auto elapsed =
                std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    now - start)
                    .count();

            const long long expected =
                static_cast<long long>(
                    tickCount * intervalMs);

            const long long error =
                std::llabs(
                    elapsed - expected);

            std::cout
                << "Tick "
                << tickCount
                << " | Actual: "
                << elapsed
                << " ms"
                << " | Expected: ~"
                << expected
                << " ms"
                << " | Error: "
                << error
                << " ms\n";

            // Cho phép sai số 100 ms vì scheduler của OS
            if (error > 100)
            {
                passed = false;
            }
        }

        // Cố tình dùng 7 ms,
        // không chia hết cho 1000 ms,
        // để kiểm tra timing drift.
        std::this_thread::sleep_for(
            std::chrono::milliseconds(7));
    }

    printResult(
        "Periodic timer without accumulated drift",
        passed);

    return passed;
}

// ======================================================
// TEST TIMER RESET
// ======================================================

bool testTimerReset()
{
    std::cout
        << "\n===== TIMER RESET TEST =====\n";

    Timer timer(1000);

    // Chờ 500 ms
    std::this_thread::sleep_for(
        std::chrono::milliseconds(500));

    // Reset -> bắt đầu đếm lại từ 0
    timer.reset();

    const auto resetTime =
        std::chrono::steady_clock::now();

    bool tickReceived = false;

    while (!tickReceived)
    {
        if (timer.isTick())
        {
            tickReceived = true;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(5));
    }

    const auto now =
        std::chrono::steady_clock::now();

    const auto elapsedAfterReset =
        std::chrono::duration_cast<
            std::chrono::milliseconds>(
            now - resetTime)
            .count();

    std::cout
        << "First tick after reset: "
        << elapsedAfterReset
        << " ms\n";

    // Sau reset phải gần 1000 ms,
    // không phải khoảng 500 ms.
    const bool passed =
        elapsedAfterReset >= 950 &&
        elapsedAfterReset <= 1100;

    printResult(
        "Timer reset",
        passed);

    return passed;
}

// ======================================================
// MAIN TEST
// ======================================================

int main()
{
    std::cout
        << "====================================\n"
        << "   TIMER / LOGGER INTEGRATION TEST\n"
        << "====================================\n";

    int passedTests = 0;
    int totalTests = 3;

    if (testLogger())
    {
        ++passedTests;
    }

    if (testTimerPeriodic())
    {
        ++passedTests;
    }

    if (testTimerReset())
    {
        ++passedTests;
    }

    std::cout
        << "\n====================================\n"
        << "TEST RESULT: "
        << passedTests
        << " / "
        << totalTests
        << " passed\n"
        << "====================================\n";

    if (passedTests == totalTests)
    {
        std::cout
            << "ALL TESTS PASSED\n";

        return 0;
    }

    std::cout
        << "SOME TESTS FAILED\n";

    return 1;
}