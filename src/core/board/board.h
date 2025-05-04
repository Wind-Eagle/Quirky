#ifndef QUIRKY_SRC_CORE_POSITION_BOARD_H
#define QUIRKY_SRC_CORE_POSITION_BOARD_H

#include <string>
#include "types.h"

namespace q_core {

constexpr coord_t NO_ENPASSANT_COORD = 0;

struct Board {
  public:
    bitboard_t bb_pieces[NUMBER_OF_CELLS];
    bitboard_t bb_colors[2];
    cell_t cells[BOARD_SIZE];
    hash_t hash;
    coord_t en_passant_coord;
    Castling castling;
    uint8_t fifty_rule_move_count;
    uint16_t move_count;
    Color move_side;
    int16_t info;

    enum class FENParseStatus : int8_t {
        Ok = 0,
        InvalidNumberOfFENFields = 1,
        InvalidNumberOfRows = 2,
        InvalidNumberOfColumns = 3,
        InvalidSizeOfColumnSkip = 4,
        InvalidCell = 5,
        InvalidMoveSide = 6,
        InvalidCastling = 7,
        InvalidEnPassantCoord = 8,
        InvalidQuietMoveCount = 9,
        InvalidMoveCount = 10
    };

    Board::FENParseStatus MakeFromFEN(const std::string_view& fen);
    std::string GetFEN() const;
    bool IsValid() const;

  private:
    void MakeBitboards();
    void MakeHash();
};

}  // namespace q_core

#endif  // QUIRKY_SRC_CORE_POSITION_BOARD_H
