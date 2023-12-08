#ifndef QUIRKY_SRC_EVAL_FEATURES_FEATURE_H
#define QUIRKY_SRC_EVAL_FEATURES_FEATURE_H

#include <cstdint>

namespace q_eval {

enum class Feature : uint16_t {
    IsolatedPawn = 0,
    BackwardPawn = 1,
    OpenedPawn = 2,
    DoubledPawn = 3,
    DoubledIsolatedPawn = 4,
    DoubledPassedPawn = 5,
    PassedPawn = 6,
    PassedPawnOnRank4 = 7,
    PassedPawnOnRank5 = 8,
    PassedPawnOnRank6 = 9,
    ConnectedPawn = 10,
    ConnectedPawnOnRank4 = 11,
    ConnectedPawnOnRank5 = 12,
    ConnectedPawnOnRank6 = 13,
    ConnectedPawnOnRank7 = 14,
    DefendedPawn = 15,
    DefendedPawnOnRank4 = 16,
    DefendedPawnOnRank5 = 17,
    DefendedPawnOnRank6 = 18,
    DefendedPawnOnRank7 = 19,
    SurelyUnstoppablePawn = 20,
    PawnIslands = 21,
    NoPawns = 22,
    KnightPair = 23,
    BishopPair = 24,
    RookOnOpenFile = 25,
    RookOnHalfOpenFile = 26,
    KingPawnShield1 = 27,
    KingPawnShield2 = 28,
    KingPawnShield3 = 29,
    KingPawnShield4 = 30,
    KingPawnShield5 = 31,
    KingPawnShield6 = 32,
    KingPawnStorm1 = 33,
    KingPawnStorm2 = 34,
    KingPawnStorm3 = 35,
    KingPawnStorm4 = 36,
    KingPawnStorm5 = 37,
    KingPawnStorm6 = 38,
    QueenKingDistance1 = 39,
    QueenKingDistance2 = 40,
    QueenKingDistance3 = 41,
    QueenKingDistance4 = 42,
    Count = 43
};

constexpr q_core::coord_t PASSED_PAWN_FEATURE_FIRST = 3;
constexpr q_core::coord_t PASSED_PAWN_FEATURE_LAST = 5;
constexpr q_core::coord_t CONNECTED_PAWN_FEATURE_FIRST = 3;
constexpr q_core::coord_t CONNECTED_PAWN_FEATURE_LAST = 6;
constexpr q_core::coord_t DEFENDED_PAWN_FEATURE_FIRST = 3;
constexpr q_core::coord_t DEFENDED_PAWN_FEATURE_LAST = 6;
constexpr q_core::coord_t QUEEN_KING_DISTANCE_FEATURE_FIRST = 1;
constexpr q_core::coord_t QUEEN_KING_DISTANCE_FEATURE_LAST = 4;

constexpr uint16_t FEATURE_COUNT = static_cast<uint16_t>(Feature::Count);

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_FEATURES_FEATURE_H
