#ifndef QUIRKY_SRC_CORE_BOARD_BOARD_H
#define QUIRKY_SRC_CORE_BOARD_BOARD_H

#include <string>

#include "../../types/types.h"

namespace core {

// These constants actually should be inside the board structure,
// but it's too boring to write Board::BOARD_SIDE each time
constexpr uint8_t BOARD_SIDE = 8;
constexpr uint8_t BOARD_SIZE = BOARD_SIDE * BOARD_SIDE;

/// This structure represents a chess board. It is a compact structure (200 bytes),
/// which doesn't store extraneous information about the position.
struct Board {
  public:
    // bb_pieces[0] is storing not empty cells, but occupied cells instead.
    // It was made to make the board structure more compact.
    bitboard_t bb_pieces[NUMBER_OF_CELLS];
    bitboard_t bb_colors[2];
    cell_t cells[BOARD_SIZE];
    hash_t hash;
    coord_t en_passant_coord;
    Castling castling;
    Color move_side;
    uint8_t quiet_move_count;
    uint16_t move_count;
    // Because of structure alignment, we get additional 2 bytes
    int16_t info;

    /// This function constructs Board structure from FEN. It assumes that FEN notation is correct,
    /// and incorrect FEN notation will lead to undefined behaviour.
    void BuildFromFEN(const std::string& fen);

    /// This function constructs FEN from Board structure. It assumes that the position on the board
    /// is a correct chess position, and incorrect position will lead to undefined behaviour.
    std::string GetFEN() const;

  private:
    /// This is a private function that builds bitboards from fields that are constructed in
    /// MakeFromFEN function
    void BuildBitboards();

    /// This is a private function that builds hash from fields that are constructed in
    /// MakeFromFEN function
    void BuildHash();
};

}  // namespace core

#endif  // QUIRKY_SRC_CORE_BOARD_BOARD_H
