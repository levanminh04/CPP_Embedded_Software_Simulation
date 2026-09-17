#include "../include/traffic/Timer.h"

Timer::Timer(int intervalMs)
    : intervalMs_(intervalMs),
      lastTick_(std::chrono::steady_clock::now())
{
}

bool Timer::isTick()
{
    const auto now = std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastTick_);

    if (elapsed >= std::chrono::milliseconds(intervalMs_))
    {
        // Không dùng:
        // lastTick_ = now;
        //
        // Vì now có thể là 1005 ms, 2026 ms...
        // Nếu gán trực tiếp sẽ làm trôi mốc thời gian.

        lastTick_ += std::chrono::milliseconds(intervalMs_);

        return true;
    }

    return false;
}

void Timer::reset()
{
    lastTick_ = std::chrono::steady_clock::now();
}