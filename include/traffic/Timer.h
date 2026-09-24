#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer
{
public:
    explicit Timer(int intervalMs = 1000);

    // Compatibility API: consume at most one pending tick.
    // Useful for simple polling loops.
    bool isTick();

    // Consume all ticks that became due since the previous time marker.
    // This prevents lost simulation time if the super loop is delayed.
    int consumeTicks();

    // Restart the timer from the current steady_clock time.
    void reset();

private:
    std::chrono::milliseconds interval_;
    std::chrono::steady_clock::time_point lastTick_;
};

#endif
