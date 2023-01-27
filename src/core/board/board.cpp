#include "board.h"

#include <vector>

#include "../../util/bit.h"
#include "../../util/geometry.h"
#include "../../util/string.h"
#include "hash.h"

namespace core {

void Board::BuildFromFEN(const std::string& fen) {
    std::vector<std::string> parsed_fen = util::SplitString(fen);
    std::vector<std::string> parsed_position = util::SplitString(parsed_fen[0], '/');
    for (subcoord_t i = 0; i < BOARD_SIDE; i++) {
        // Clearing current position
        for (subcoord_t j = 0; j < BOARD_SIDE; j++) {
            cells[util::MakeCoord(util::InvertCoord(i), j)] = EMPTY_CELL;
        }
        subcoord_t j = 0;
        for (auto c : parsed_position[i]) {
            if (isdigit(c)) {
                j += c - '0';
            } else {
                cells[util::MakeCoord(util::InvertCoord(i), j)] = util::CharToCell(c);
                j++;
            }
        }
    }
    move_side = util::CharToColor(parsed_fen[1][0]);
    castling = util::StringToCastling(parsed_fen[2]);
    en_passant_coord = util::StringToCoord(parsed_fen[3]);
    quiet_move_count = stoi(parsed_fen[4]);
    move_count = stoi(parsed_fen[5]) * 2 - (move_side == Color::White ? 1 : 0);
    BuildBitboards();
    BuildHash();
}

std::string Board::GetFEN() const {
    std::string res;
    for (subcoord_t i = 0; i < BOARD_SIDE; i++) {
        uint8_t block_size = 0;
        for (subcoord_t j = 0; j < BOARD_SIDE; j++) {
            coord_t cur = util::MakeCoord(util::InvertCoord(i), j);
            if (cells[cur] == EMPTY_CELL) {
                block_size++;
            } else {
                if (block_size > 0) {
                    res += static_cast<char>(block_size + '0');
                }
                res += util::CellToChar(cells[cur]);
                block_size = 0;
            }
        }
        if (block_size > 0) {
            res += static_cast<char>(block_size + '0');
        }
    }
    res += {' ', util::ColorToChar(move_side)};
    res += " " + util::StringToCastling(castling);
    res += " " + util::CoordToString(en_passant_coord);
    res += " " + std::to_string(quiet_move_count);
    res += " " + std::to_string((move_count + 1) / 2);
    return res;
}

void Board::BuildBitboards() {
    // Clearing current bitboards
    for (int8_t i = 0; i < NUMBER_OF_CELLS; i++) {
        bb_pieces[i] = 0;
    }
    bb_colors[0] = 0;
    bb_colors[1] = 0;
    for (coord_t i = 0; i < 64; i++) {
        // bb_pieces[0] stores bitboard of all occupied cells, so it should be built another way
        if (cells[i] != EMPTY_CELL) {
            util::SetBit(bb_pieces[cells[i]], i);
            util::SetBit(bb_pieces[0], i);
        }
        util::SetBit(bb_colors[static_cast<int8_t>(util::GetCellColor(cells[i]))], i);
    }
}

void Board::BuildHash() {
    // Clearing current hash
    hash = 0;
    for (coord_t i = 0; i < 64; i++) {
        hash ^= MakeZobristHashFromCell(i, cells[i]);
    }
    hash ^= MakeZobristHashFromEnPassantCoord(en_passant_coord);
    hash ^= MakeZobristHashFromCastling(castling);
    hash ^= MakeZobristHashFromMoveSide(move_side);
}

}  // namespace core