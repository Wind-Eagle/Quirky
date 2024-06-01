#include "time.h"

#include <chrono>

namespace q_search {

time_t GetMaxTime(const GameTimeControl& time_control) {
    return time_control.white_time.time;
}

time_t GetMaxTime(const FixedTimeControl& time_control) {
    return time_control.time;
}

time_t GetMaxTime(const InfiniteTimeControl& time_control) {
    return TIME_INF;
}

SearchTimer::SearchTimer(time_control_t time_control, SearchControl& control, SearchStat& stat)
    : time_control_(time_control), control_(control), stat_(stat) {
    max_time_ = std::visit([this](const auto& time_control) { return GetMaxTime(time_control); },
                           time_control_);
    start_time_ = std::chrono::steady_clock::now();
}

void SearchTimer::Start() { start_time_ = std::chrono::steady_clock::now(); }

std::chrono::milliseconds SearchTimer::GetWaitTime() const {
    static constexpr time_t TICK_TIME = 30;
    auto time_since_start = GetTimeSinceStart();
    if (time_since_start >= max_time_) {
        return std::chrono::milliseconds(0);
    }
    return std::chrono::milliseconds(std::min(max_time_ - time_since_start, TICK_TIME));
}

time_t SearchTimer::GetTimeSinceStart() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time_).count();
}

}  // namespace q_search
