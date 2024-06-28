#include "time.h"

#include <chrono>

namespace q_search {

static constexpr time_t TICK_TIME = 30;

time_t GetMaxTime(const FixedTimeControl& time_control, const Position& position) {
    return time_control.time;
}

time_t GetMaxTime(const InfiniteTimeControl& time_control, const Position& position) {
    return TIME_INF;
}

time_t GetMaxTime(const GameTimeControl& time_control, const Position& position) {
    const PlayerTime player_time =
        (position.board.move_side == q_core::Color::White ? time_control.white_time
                                                          : time_control.black_time);
    time_t max_time = 0;
    float opening_factor = 2 - std::min(position.board.move_count, static_cast<uint16_t>(20)) / 20.0;
    if (time_control.moves_to_go != GameTimeControl::NO_MOVES_TO_GO) {
        max_time = player_time.time / time_control.moves_to_go * opening_factor + player_time.increment;
        max_time = std::min(max_time, player_time.time - 1);
    } else {
        float no_time_factor = player_time.time < 200 ? 5 : player_time.time < 1000 ? 2 : 1;
        uint16_t moves_to_go_imitation = std::max(5, 20 - position.board.move_count / 5);
        max_time = player_time.time / moves_to_go_imitation * opening_factor / no_time_factor + player_time.increment;
        max_time = std::min(max_time, player_time.time / 2);
    }
    return max_time;
}

SearchTimer::SearchTimer(time_control_t time_control, SearchControl& control, SearchStat& stat,
                         Position& position)
    : time_control_(time_control), control_(control), stat_(stat), position_(position) {
    start_time_ = std::chrono::steady_clock::now();
}

void SearchTimer::ProcessNextDepth() {
    const auto time_since_start = GetTimeSinceStart();
    const auto estimated_time_left = context_.estimated_max_time - time_since_start;
    if (estimated_time_left < time_since_start) {
        context_.should_stop = true;
    }
}

std::chrono::milliseconds SearchTimer::GetWaitTime() {
    if (context_.should_stop) {
        return std::chrono::milliseconds(0);
    }
    context_.estimated_max_time =
        std::visit([this](const auto& time_control) { return GetMaxTime(time_control, position_); },
                   time_control_);
    auto time_since_start = GetTimeSinceStart();
    if (time_since_start >= context_.estimated_max_time) {
        return std::chrono::milliseconds(0);
    }
    return std::chrono::milliseconds(std::min(context_.estimated_max_time - time_since_start, TICK_TIME));
}

time_t SearchTimer::GetTimeSinceStart() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                 start_time_)
        .count();
}

}  // namespace q_search
