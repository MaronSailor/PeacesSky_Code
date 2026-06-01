// Game Engine Timer Class

#pragma once
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>

class Timer {
private:
    // --- Member Variables ---
    mutable std::recursive_mutex m_mutex;
    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_end;
    std::chrono::high_resolution_clock::time_point m_lastTick;

    bool m_running;
    bool m_paused;
    std::atomic<bool> m_cancelled;

    // Atomic so getDeltaTime() is safe to call from any thread (Core 2/3/4)
    std::atomic<double> m_deltaTime;

    std::function<void()> m_callback = nullptr;
    double m_callbackTime = 0.0;

public:
    // --- Constructor ---
    Timer()
        : m_running(false),
        m_paused(false),
        m_cancelled(false),
        m_deltaTime(0.0) {
    }

    ~Timer() {
        cancel();
    }



    // --- Basic Functions ---
    std::chrono::time_point<std::chrono::high_resolution_clock> createTimePoint();

    void start();

    void stop();

    void reset();

    void restart();

    double getElapsedMilliseconds() const;

    double getElapsedSeconds() const;

    double tick();

    // --- Pause / Resume ---
    void pause();

    void resume();

    bool isPaused() const;

    // --- DeltaTime ---
    double getDeltaTime() const;

    void update();

    bool isElapsed(double ms) const;

    // --- Callback ---
    template<typename Func, typename... Args>
    void setCallback(double ms, Func&& handler, Args&&... args);

    // --- Asynchronous Start ---
    template<typename Func, typename... Args>
    void startAsync(double ms, Func&& handler, Args&&... args);

    // --- Periodic Async Callback ---
    template<typename Func, typename... Args>
    void startPeriodicAsync(double ms, Func&& handler, Args&&... args);

    void cancel();

    // --- Delay ---
    static void delay(double milliseconds);

    // --- DelayPrecise (CPU-heavy for short delays) ---
    static void delayPrecise(double milliseconds);
};


template<typename Func, typename ...Args>
inline void Timer::setCallback(double ms, Func&& handler, Args&& ...args) {
    std::lock_guard<std::recursive_mutex> lock(this->m_mutex);
    this->m_callback = [handler = std::forward<Func>(handler), ...args = std::forward<Args>(args)]() mutable {
        std::invoke(handler, std::forward<Args>(args)...);
    };
    this->m_callbackTime = ms;
}

// --- Asynchronous Start ---
template<typename Func, typename... Args>
inline void Timer::startAsync(double ms, Func&& handler, Args&&... args) {
    this->m_cancelled = false;
    std::thread([this, ms, handler, ...args = std::forward<Args>(args)]() mutable {
        auto start = std::chrono::steady_clock::now();
        while (!this->m_cancelled) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(now - start).count();
            if (elapsed >= ms) {
                if (!this->m_cancelled) std::invoke(handler, std::forward<Args>(args)...);
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }).detach();
}

// --- Periodic Async Callback ---
template<typename Func, typename... Args>
inline void Timer::startPeriodicAsync(double ms, Func&& handler, Args&&... args) {
    this->m_cancelled = false;
    std::thread([this, ms, handler, ...args = std::forward<Args>(args)]() mutable {
        while (!this->m_cancelled) {
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(ms));
            if (!this->m_cancelled) std::invoke(handler, std::forward<Args>(args)...);
        }
        }).detach();
}
