#include "../include/traffic/Timer.h"

#include <limits>
#include <stdexcept>

Timer::Timer(int intervalMs)
    : interval_(intervalMs),
      lastTick_(std::chrono::steady_clock::now())
{
    if (intervalMs <= 0)
    {
        throw std::invalid_argument("Timer interval must be greater than 0 ms");
    }
}

bool Timer::isTick()
{
    const auto now = std::chrono::steady_clock::now();

    if (now - lastTick_ < interval_)
    {
        return false;
    }

    // Advance by the configured interval instead of assigning now.
    // This keeps the periodic schedule anchored and avoids accumulated drift.
    lastTick_ += interval_;
    return true;
}

int Timer::consumeTicks()
{
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick_);

    if (elapsed < interval_)
    {
        return 0;
    }

    const auto tickCount = elapsed.count() / interval_.count();

    lastTick_ += interval_ * tickCount;

    if (tickCount > std::numeric_limits<int>::max())
    {
        return std::numeric_limits<int>::max();
    }

    return static_cast<int>(tickCount);
}

void Timer::reset()
{
    lastTick_ = std::chrono::steady_clock::now();
}
