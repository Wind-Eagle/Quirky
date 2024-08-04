#include "eval_features.h"

#include <array>

#include "../core/board/geometry.h"
#include "../core/moves/magic.h"
#include "../core/util.h"
#include "../util/bit.h"
#include "../util/math.h"
#include "model.h"

using namespace q_core;

namespace q_eval {

struct HelperBitboards {
    // Color dependent
    std::array<std::array<bitboard_t, BOARD_SIZE>, 2> frontspan;
    std::array<std::array<bitboard_t, BOARD_SIZE>, 2> passed_enemies;
    std::array<std::array<bitboard_t, BOARD_SIZE>, 2> connected;
    // Color independent
    std::array<bitboard_t, BOARD_SIZE> neighbours;
};

constexpr void AddToFrontspanBitboard(HelperBitboards& ans, subcoord_t rank, subcoord_t file,
                            subcoord_t new_rank, subcoord_t new_file, coord_t coord,
                            coord_t new_coord) {
    if (file == new_file) {
        if (new_rank > rank) {
            q_util::SetBit(ans.frontspan[static_cast<uint8_t>(Color::White)][coord], new_coord);
        } else if (new_rank < rank) {
            q_util::SetBit(ans.frontspan[static_cast<uint8_t>(Color::Black)][coord], new_coord);
        }
    }
}

constexpr void AddToPassedEnemiesBitboard(HelperBitboards& ans, subcoord_t rank, subcoord_t file,
                            subcoord_t new_rank, subcoord_t new_file, coord_t coord,
                            coord_t new_coord) {
    if (q_util::Abs(file - new_file) <= 1) {
        if (new_rank > rank) {
            q_util::SetBit(ans.passed_enemies[static_cast<uint8_t>(Color::White)][coord], new_coord);
        } else if (new_rank < rank) {
            q_util::SetBit(ans.passed_enemies[static_cast<uint8_t>(Color::Black)][coord], new_coord);
        }
    }
}

constexpr void AddToConnectedBitboard(HelperBitboards& ans, subcoord_t rank, subcoord_t file,
                            subcoord_t new_rank, subcoord_t new_file, coord_t coord,
                            coord_t new_coord) {
    if (q_util::Abs(file - new_file) == 1) {
        if (new_rank == rank || new_rank + 1 == rank) {
            q_util::SetBit(ans.connected[static_cast<uint8_t>(Color::White)][coord], new_coord);
        }
        if (new_rank == rank || new_rank == rank + 1) {
            q_util::SetBit(ans.connected[static_cast<uint8_t>(Color::Black)][coord], new_coord);
        }
    }
}

constexpr void AddToNeighboursBitboard(HelperBitboards& ans, subcoord_t, subcoord_t file,
                            subcoord_t, subcoord_t new_file, coord_t coord,
                            coord_t new_coord) {
    if (q_util::Abs(file - new_file) == 1) {
        q_util::SetBit(ans.neighbours[coord], new_coord);
    }
}

constexpr HelperBitboards MakeHelperBitboards() {
    HelperBitboards ans{};
    for (coord_t coord = 0; coord < BOARD_SIZE; coord++) {
        for (coord_t new_coord = 0; new_coord < BOARD_SIZE; new_coord++) {
            const subcoord_t rank = GetRank(coord);
            const subcoord_t file = GetFile(coord);
            const subcoord_t new_rank = GetRank(new_coord);
            const subcoord_t new_file = GetFile(new_coord);
            AddToFrontspanBitboard(ans, rank, file, new_rank, new_file, coord, new_coord);
            AddToPassedEnemiesBitboard(ans, rank, file, new_rank, new_file, coord, new_coord);
            AddToConnectedBitboard(ans, rank, file, new_rank, new_file, coord, new_coord);
            AddToNeighboursBitboard(ans, rank, file, new_rank, new_file, coord, new_coord);
        }
    }
    return ans;
}

constexpr HelperBitboards HELPER_BITBOARDS = MakeHelperBitboards();

bool IsPawnIsolated(const PawnContext& context) {
    return !(HELPER_BITBOARDS.neighbours[GetFile(context.pawn_coord)] & context.our_pawns);
}

bool IsPawnDoubled(const PawnContext& context) {
    return HELPER_BITBOARDS.frontspan[static_cast<uint8_t>(context.color)][context.pawn_coord] & context.our_pawns;
}

}  // namespace q_eval