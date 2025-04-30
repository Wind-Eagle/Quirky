#include "time.h"

#include <chrono>

namespace q_search {

static constexpr time_t TICK_TIME = 30;

static constexpr time_t NO_SLOWING_TIME_THRESHOLD = 10;

time_t SearchTimer::GetMaxTime(const FixedTimeControl& time_control) const {
    return time_control.time;
}

time_t SearchTimer::GetMaxTime(const InfiniteTimeControl&) const { return TIME_INF; }

time_t SearchTimer::GetMaxTime(const GameTimeControl& time_control) const {
    const PlayerTime player_time =
        (position_.board.move_side == q_core::Color::White ? time_control.white_time
                                                           : time_control.black_time);
    time_t max_time = 0;
    if (time_control.moves_to_go != GameTimeControl::NO_MOVES_TO_GO) {
        size_t moves_count = time_control.moves_to_go / 2 + 1;
        if (time_control.moves_to_go <= 3) {
            moves_count = time_control.moves_to_go;
        }
        max_time = player_time.time / moves_count +
                   player_time.increment;
        max_time = std::min(max_time, player_time.time - time_control.moves_to_go * 50);
    } else {
        float no_time_factor = player_time.time < 200 ? 5 : player_time.time < 1000 ? 2 : 1;
        uint16_t moves_to_go_imitation = std::max(5, 20 - position_.board.move_count / 5);
        max_time =
            player_time.time / moves_to_go_imitation / no_time_factor +
            player_time.increment;
        max_time = std::min(max_time, player_time.time / 2);
    }
    return max_time;
}

void SearchTimer::UpdateOnNextDepth(const FixedTimeControl&) {}
void SearchTimer::UpdateOnNextDepth(const InfiniteTimeControl&) {}
void SearchTimer::UpdateOnNextDepth(const GameTimeControl&) {
    const auto time_since_start = GetTimeSinceStart();
    const auto estimated_time_left = context_.estimated_max_time - time_since_start;
    if (estimated_time_left < time_since_start) {
        context_.should_stop = true;
    }
    if (context_.best_moves.size() == 1 && estimated_time_left < time_since_start * 2) {
        context_.should_stop = true;
    }
}

SearchTimer::SearchTimer(time_control_t time_control, Position& position)
    : time_control_(time_control), position_(position) {
    start_time_ = std::chrono::steady_clock::now();
}

void SearchTimer::ProcessNextDepth(const SearchResult& result) {
    // Update timing statistics
    const auto time_since_start = GetTimeSinceStart();
    const uint16_t cur_move = q_core::GetCompressedMove(result.best_move);
    if (cur_move != context_.last_move && time_since_start > NO_SLOWING_TIME_THRESHOLD) {
        context_.changed_last_move = true;
    }
    context_.best_moves.insert(cur_move);
    context_.last_move = cur_move;
    context_.last_score = result.score;
    context_.estimated_max_time = std::visit(
        [this](const auto& time_control) { return GetMaxTime(time_control); }, time_control_);

    // Update context
    std::visit([this](const auto& time_control) { UpdateOnNextDepth(time_control); },
               time_control_);
}

std::chrono::milliseconds SearchTimer::GetWaitTime() {
    if (context_.should_stop) {
        return std::chrono::milliseconds(0);
    }
    context_.estimated_max_time = std::visit(
        [this](const auto& time_control) { return GetMaxTime(time_control); }, time_control_);
    auto time_since_start = GetTimeSinceStart();
    if (time_since_start >= context_.estimated_max_time) {
        return std::chrono::milliseconds(0);
    }
    return std::chrono::milliseconds(
        std::min(context_.estimated_max_time - time_since_start, TICK_TIME));
}

time_t SearchTimer::GetTimeSinceStart() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                 start_time_)
        .count();
}

}  // namespace q_search
