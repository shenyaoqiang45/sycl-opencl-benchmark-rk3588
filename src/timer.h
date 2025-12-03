#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
public:
    Timer() : m_start(), m_end() {}

    void start() {
        m_start = std::chrono::high_resolution_clock::now();
    }

    double stop() {
        m_end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start);
        return duration.count() / 1000.0; // Return milliseconds
    }

private:
    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_end;
};

#endif // TIMER_H
