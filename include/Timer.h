#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer
{
public:
    explicit Timer(int intervalMs = 1000);

    // Trả về true khi đã đủ 1 chu kỳ
    bool isTick();

    // Reset lại thời gian
    void reset();

private:
    int intervalMs_;

    std::chrono::steady_clock::time_point lastTick_;
};

#endif