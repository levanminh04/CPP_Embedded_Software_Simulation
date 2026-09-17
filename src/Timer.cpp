#include "../include/Timer.h"

Timer::Timer(int intervalMs)
    : intervalMs_(intervalMs),
      lastTick_(std::chrono::steady_clock::now())
{
}

bool Timer::isTick()
{
    auto now = std::chrono::steady_clock::now();

    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastTick_)
            .count();

    if (elapsed >= intervalMs_)
    {
        lastTick_ = now;
        return true;
    }

    return false;
}

void Timer::reset()
{
    lastTick_ = std::chrono::steady_clock::now();
}