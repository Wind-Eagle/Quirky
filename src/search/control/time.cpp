#include "time.h"

#include <chrono>

namespace q_search {

static constexpr time_t TICK_TIME = 30;

time_t SearchTimer::GetSoftTime(const FixedTimeControl& time_control) const {
    return time_control.time;
}

time_t SearchTimer::GetSoftTime(const InfiniteTimeControl&) const { return TIME_INF; }

time_t SearchTimer::GetSoftTime(const GameTimeControl& time_control) const {
    const PlayerTime player_time =
        (position_.board.move_side == q_core::Color::White ? time_control.white_time
                                                           : time_control.black_time);
    time_t soft_time = 0;
    if (time_control.moves_to_go != GameTimeControl::NO_MOVES_TO_GO) {
        soft_time = player_time.time / time_control.moves_to_go * 0.5 + player_time.increment;
        soft_time = std::min(soft_time, player_time.time - time_control.moves_to_go * 50);
    } else {
        float no_time_factor = player_time.time < 200 ? 5 : player_time.time < 1000 ? 2 : 1;
        uint16_t moves_to_go_imitation = 30;
        soft_time = player_time.time / moves_to_go_imitation / no_time_factor * 0.5 * 0.6 +
                    player_time.increment;
        soft_time = std::min(soft_time, player_time.time / 2);
    }
    return soft_time;
}

time_t SearchTimer::GetMaxTime(const FixedTimeControl&, time_t soft_time) const {
    return soft_time;
}

time_t SearchTimer::GetMaxTime(const InfiniteTimeControl&, time_t soft_time) const {
    return soft_time;
}

time_t SearchTimer::GetMaxTime(const GameTimeControl& time_control, time_t soft_time) const {
    const PlayerTime player_time =
        (position_.board.move_side == q_core::Color::White ? time_control.white_time
                                                           : time_control.black_time);
    if (time_control.moves_to_go != GameTimeControl::NO_MOVES_TO_GO) {
        return std::min(soft_time * std::min(6, (time_control.moves_to_go + 3) / 2),
                        player_time.time - time_control.moves_to_go * 50);
    }
    return std::min(soft_time * 6, player_time.time / 2);
}

static constexpr std::array<float, 5> PV_STABILITY_FACTOR = {2.5, 1.2, 0.9, 0.8, 0.75};

void SearchTimer::UpdateOnNextDepth(const FixedTimeControl&) {}
void SearchTimer::UpdateOnNextDepth(const InfiniteTimeControl&) {}
void SearchTimer::UpdateOnNextDepth(const GameTimeControl&) {
    const auto time_since_start = GetTimeSinceStart();
    if (context_.estimated_max_time <= time_since_start * 1.6) {
        context_.should_stop = true;
        return;
    }
    float time_adjust_factor = 1;
    time_adjust_factor *=
        PV_STABILITY_FACTOR[std::min(context_.pv_stability, static_cast<uint16_t>(4))];
    float node_frac =
        static_cast<float>(stat_.GetNodesCount(context_.last_move)) / stat_.GetNodesCount();
    float node_factor = (1.5 - node_frac) * 1.75;
    time_adjust_factor *= node_factor;

    if (time_since_start >= context_.estimated_soft_time * time_adjust_factor) {
        context_.should_stop = true;
    }
}

SearchTimer::SearchTimer(time_control_t time_control, Position& position, SearchStat& stat)
    : time_control_(time_control), position_(position), stat_(stat) {
    start_time_ = std::chrono::steady_clock::now();
}

void SearchTimer::ProcessNextDepth(const SearchResult& result) {
    // Update timing statistics
    const uint16_t cur_move = q_core::GetCompressedMove(result.best_move);
    if (cur_move == context_.last_move) {
        context_.pv_stability++;
    } else {
        context_.pv_stability = 0;
    }
    context_.depth = result.depth;
    context_.last_move = cur_move;
    context_.estimated_soft_time = std::visit(
        [this](const auto& time_control) { return GetSoftTime(time_control); }, time_control_);
    context_.estimated_max_time = std::visit(
        [this](const auto& time_control) {
            return GetMaxTime(time_control, context_.estimated_soft_time);
        },
        time_control_);

    // Update context
    std::visit([this](const auto& time_control) { UpdateOnNextDepth(time_control); },
               time_control_);
}

std::chrono::milliseconds SearchTimer::GetWaitTime() {
    if (context_.should_stop) {
        return std::chrono::milliseconds(0);
    }
    context_.estimated_soft_time = std::visit(
        [this](const auto& time_control) { return GetSoftTime(time_control); }, time_control_);
    context_.estimated_max_time = std::visit(
        [this](const auto& time_control) {
            return GetMaxTime(time_control, context_.estimated_soft_time);
        },
        time_control_);
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
