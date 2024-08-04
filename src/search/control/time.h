#ifndef QUIRKY_SRC_SEARCH_CONTROL_TIME_H
#define QUIRKY_SRC_SEARCH_CONTROL_TIME_H

#include <chrono>
#include <cstdint>
#include <limits>
#include <variant>

#include "control.h"
#include "stat.h"

namespace q_search {

using time_t = uint64_t;

static inline constexpr time_t TIME_INF = std::numeric_limits<time_t>::max();

struct PlayerTime {
    time_t time;
    time_t increment;
};

struct GameTimeControl {
    PlayerTime white_time;
    PlayerTime black_time;
    uint8_t moves_to_go = NO_MOVES_TO_GO;
    static constexpr uint8_t NO_MOVES_TO_GO = 255;
};

struct FixedTimeControl {
    time_t time;
};

struct InfiniteTimeControl {};

using time_control_t = std::variant<GameTimeControl, FixedTimeControl, InfiniteTimeControl>;

class SearchTimer {
  public:
    SearchTimer(time_control_t time_control, Position& position);
    void ProcessNextDepth();
    std::chrono::milliseconds GetWaitTime();
    time_t GetTimeSinceStart() const;

  private:
    struct Context {
        time_t estimated_max_time = 0;
        bool should_stop = false;
    };
    Context context_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    const time_control_t time_control_;
    const Position& position_;
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_CONTROL_TIME_H
