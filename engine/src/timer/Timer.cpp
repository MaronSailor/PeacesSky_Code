#include "Timer.hpp"

std::chrono::time_point<std::chrono::steady_clock> Timer::createTimePoint() {
    return std::chrono::steady_clock::now();
}

void Timer::start() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    this->m_start = std::chrono::steady_clock::now();
    this->m_lastTick = this->m_start;
    this->m_running = true;
    this->m_paused = false;
    this->m_cancelled = false;
}

void Timer::stop() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    this->m_end = std::chrono::steady_clock::now();
    this->m_running = false;
}

void Timer::reset() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    this->m_start = std::chrono::steady_clock::now();
    this->m_lastTick = this->m_start;
    this->m_end = this->m_start;
    this->m_paused = false;
    this->m_running = false;
    this->m_cancelled = false;
}

void Timer::restart() {
    this->reset();
    this->start();
}

double Timer::getElapsedMilliseconds() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    auto now = (this->m_running && !this->m_paused)
        ? std::chrono::steady_clock::now()
        : this->m_end;
    return std::chrono::duration<double, std::milli>(now - this->m_start).count();
}

double Timer::getElapsedSeconds() const {
    return this->getElapsedMilliseconds() / 1000.0;
}

double Timer::tick() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (!this->m_running) this->start();
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(now - this->m_lastTick).count();
    this->m_lastTick = now;
    return elapsed;
}

// --- Pause / Resume ---
void Timer::pause() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (this->m_running && !this->m_paused) {
        this->m_end = std::chrono::steady_clock::now();
        this->m_paused = true;
    }
}

void Timer::resume() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (this->m_running && this->m_paused) {
        auto pauseDuration = std::chrono::steady_clock::now() - this->m_end;
        this->m_start += pauseDuration; // shift start time
        this->m_lastTick = std::chrono::steady_clock::now();
        this->m_paused = false;
    }
}

bool Timer::isPaused() const {
    return this->m_paused;
}

// --- DeltaTime ---
double Timer::getDeltaTime() const {
    // Relaxed load: readers on other cores get a consistent double value;
    // no ordering guarantee needed since delta time is independently consumed.
    return this->m_deltaTime.load(std::memory_order_relaxed);
}

void Timer::update() {
    // Lock only to guard m_lastTick (a non-atomic time_point).
    // The delta is then stored atomically so any thread can read it safely.
    const double delta = [&]
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto now = std::chrono::steady_clock::now();
        const double d = std::chrono::duration<double, std::milli>(now - this->m_lastTick).count();
        this->m_lastTick = now;
        return d;
    }();

    this->m_deltaTime.store(delta, std::memory_order_relaxed);

    if (this->m_callback && this->m_running && !this->m_paused) {
        double elapsed = this->getElapsedMilliseconds();
        if (elapsed >= this->m_callbackTime) {
            this->m_callback();
            this->m_callback = nullptr; // single-use
        }
    }
}

bool Timer::isElapsed(double ms) const {
    return this->getElapsedMilliseconds() >= ms;
}

void Timer::cancel() {
    this->m_cancelled = true;
}

// --- Delay ---
void Timer::delay(double milliseconds) {
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(milliseconds));
}

// --- DelayPrecise (CPU-heavy for short delays) ---
void Timer::delayPrecise(double milliseconds) {
    auto start = std::chrono::steady_clock::now();
    if (milliseconds > 20) {
        delay(milliseconds);
        return;
    }
    while (true) {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - start).count();
        if (elapsed >= milliseconds) break;
        std::this_thread::yield();
    }
}