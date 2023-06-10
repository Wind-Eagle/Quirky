#include "board.h"

#include <vector>

#include "../../util/bit.h"
#include "../../util/string.h"
#include "geometry.h"
#include "hash.h"
#include "util.h"

namespace q_core {

inline constexpr cell_t CastCharToCell(char c) {
    switch (c) {
        case 'P':
            return MakeCell(Color::White, Piece::Pawn);
        case 'N':
            return MakeCell(Color::White, Piece::Knight);
        case 'B':
            return MakeCell(Color::White, Piece::Bishop);
        case 'R':
            return MakeCell(Color::White, Piece::Rook);
        case 'Q':
            return MakeCell(Color::White, Piece::Queen);
        case 'K':
            return MakeCell(Color::White, Piece::King);
        case 'p':
            return MakeCell(Color::Black, Piece::Pawn);
        case 'n':
            return MakeCell(Color::Black, Piece::Knight);
        case 'b':
            return MakeCell(Color::Black, Piece::Bishop);
        case 'r':
            return MakeCell(Color::Black, Piece::Rook);
        case 'q':
            return MakeCell(Color::Black, Piece::Queen);
        case 'k':
            return MakeCell(Color::Black, Piece::King);
        case '.':
            return EMPTY_CELL;
        default:
            return UNDEFINED_CELL;
    }
}

inline constexpr char CastCellToChar(const cell_t c) {
    Q_ASSERT(IsCellValid(c));
    switch (c) {
        case MakeCell(Color::White, Piece::Pawn):
            return 'P';
        case MakeCell(Color::White, Piece::Knight):
            return 'N';
        case MakeCell(Color::White, Piece::Bishop):
            return 'B';
        case MakeCell(Color::White, Piece::Rook):
            return 'R';
        case MakeCell(Color::White, Piece::Queen):
            return 'Q';
        case MakeCell(Color::White, Piece::King):
            return 'K';
        case MakeCell(Color::Black, Piece::Pawn):
            return 'p';
        case MakeCell(Color::Black, Piece::Knight):
            return 'n';
        case MakeCell(Color::Black, Piece::Bishop):
            return 'b';
        case MakeCell(Color::Black, Piece::Rook):
            return 'r';
        case MakeCell(Color::Black, Piece::Queen):
            return 'q';
        case MakeCell(Color::Black, Piece::King):
            return 'k';
        case EMPTY_CELL:
            return '.';
        default:
            return '?';
    }
}

inline constexpr Color CastCharToColor(const char c) {
    return c == 'w' ? Color::White : Color::Black;
}

inline constexpr char CastColorToChar(const Color c) {
    Q_ASSERT(IsColorValid(c));
    return c == Color::White ? 'w' : 'b';
}

inline Castling CastStringToCastling(const std::string& str) {
    Castling res = Castling::None;
    if (str.find('k') != std::string::npos) {
        res |= Castling::BlackKingSide;
    }
    if (str.find('K') != std::string::npos) {
        res |= Castling::WhiteKingSide;
    }
    if (str.find('q') != std::string::npos) {
        res |= Castling::BlackQueenSide;
    }
    if (str.find('Q') != std::string::npos) {
        res |= Castling::WhiteQueenSide;
    }
    return res;
}

inline std::string CastStringToCastling(const Castling c) {
    Q_ASSERT(IsCastlingValid(c));
    std::string res;
    if (IsCastlingAllowed(c, Castling::WhiteKingSide)) {
        res += "K";
    }
    if (IsCastlingAllowed(c, Castling::WhiteQueenSide)) {
        res += "Q";
    }
    if (IsCastlingAllowed(c, Castling::BlackKingSide)) {
        res += "k";
    }
    if (IsCastlingAllowed(c, Castling::BlackQueenSide)) {
        res += "q";
    }
    if (res.empty()) {
        res = "-";
    }
    return res;
}

inline bool IsMoveSideStringValid(const std::string& str) {
    return str == "w" || str == "b";
}

inline bool IsCastlingStringValid(const std::string& str) {
    if (str.empty() || str.size() > 4) {
        return false;
    }
    if (str == "-") {
        return true;
    }
    for (size_t i = 0; i < str.size(); i++) {
        if (!(str[i] == 'k' || str[i] == 'q' || str[i] == 'K' || str[i] == 'Q')) {
            return false;
        }
        for (size_t j = i + 1; j < str.size(); j++) {
            if (str[i] == str[j]) {
                return false;
            }
        }
    }
    return true;
}

inline bool IsEnPassantCoordStringValid(const std::string& str) {
    if (str == "-") {
        return true;
    }
    if (!IsCoordStringValid(str)) {
        return false;
    }
    return str[1] == '3' || str[1] == '6';
}

inline bool IsMoveCountStringValid(const std::string& str) {
    for (const auto i : str) {
        if (!std::isdigit(i)) {
            return false;
        }
    }
    return true;
}

#define Q_CHECK_FEN_PARSE_ERROR(condition, error) \
    do {                                      \
        if (!(condition)) {                   \
            return error;                     \
        }                                     \
    } while (false)

Board::FENParseStatus Board::MakeFromFEN(const std::string& fen) {
    std::vector<std::string> parsed_fen = q_util::SplitString(fen);
    Q_CHECK_FEN_PARSE_ERROR(parsed_fen.size() == 6, FENParseStatus::InvalidNumberOfFENFields);
    std::vector<std::string> parsed_position = q_util::SplitString(parsed_fen[0], '/');
    Q_CHECK_FEN_PARSE_ERROR(parsed_position.size() == BOARD_SIDE, FENParseStatus::InvalidNumberOfRows);
    for (subcoord_t i = 0; i < BOARD_SIDE; i++) {
        subcoord_t j = 0;
        for (auto c : parsed_position[i]) {
            Q_CHECK_FEN_PARSE_ERROR(j < BOARD_SIDE, FENParseStatus::InvalidNumberOfColumns);
            if (isdigit(c)) {
                Q_CHECK_FEN_PARSE_ERROR((c - '0') > 0 && (c - '0') <= BOARD_SIDE, FENParseStatus::InvalidSizeOfColumnSkip);
                for (coord_t cur = j; cur < j + c - '0'; cur++) {
                    cells[MakeCoord(InvertSubcoord(i), cur)] = EMPTY_CELL;
                }
                j += c - '0';
            } else {
                cell_t cell = CastCharToCell(c);
                Q_CHECK_FEN_PARSE_ERROR(cell != UNDEFINED_CELL, FENParseStatus::InvalidCell);
                cells[MakeCoord(InvertSubcoord(i), j)] = cell;
                j++;
            }
            Q_CHECK_FEN_PARSE_ERROR(j <= BOARD_SIDE, FENParseStatus::InvalidNumberOfColumns);
        }
        Q_CHECK_FEN_PARSE_ERROR(j == BOARD_SIDE, FENParseStatus::InvalidNumberOfColumns);
    }
    Q_CHECK_FEN_PARSE_ERROR(IsMoveSideStringValid(parsed_fen[1]), FENParseStatus::InvalidMoveSide);
    move_side = CastCharToColor(parsed_fen[1][0]);
    Q_CHECK_FEN_PARSE_ERROR(IsCastlingStringValid(parsed_fen[2]), FENParseStatus::InvalidCastling);
    castling = CastStringToCastling(parsed_fen[2]);
    Q_CHECK_FEN_PARSE_ERROR(IsEnPassantCoordStringValid(parsed_fen[3]), FENParseStatus::InvalidEnPassantCoord);
    en_passant_coord = CastStringToCoord(parsed_fen[3]);
    Q_CHECK_FEN_PARSE_ERROR(IsMoveCountStringValid(parsed_fen[4]), FENParseStatus::InvalidQuietMoveCount);
    quiet_move_count = stoi(parsed_fen[4]);
    Q_CHECK_FEN_PARSE_ERROR(IsMoveCountStringValid(parsed_fen[5]), FENParseStatus::InvalidMoveCount);
    move_count = stoi(parsed_fen[5]) * 2 - (move_side == Color::White ? 1 : 0);
    MakeBitboards();
    MakeHash();
    return FENParseStatus::Ok;
}

std::string Board::GetFEN() const {
    std::string res;
    for (subcoord_t i = 0; i < BOARD_SIDE; i++) {
        uint8_t block_size = 0;
        for (subcoord_t j = 0; j < BOARD_SIDE; j++) {
            coord_t cur = MakeCoord(InvertSubcoord(i), j);
            if (cells[cur] == EMPTY_CELL) {
                block_size++;
            } else {
                if (block_size > 0) {
                    res += static_cast<char>(block_size + '0');
                }
                res += CastCellToChar(cells[cur]);
                block_size = 0;
            }
        }
        if (block_size > 0) {
            res += static_cast<char>(block_size + '0');
        }
        if (i != BOARD_SIDE - 1) {
            res += "/";
        }
    }
    res += {' ', CastColorToChar(move_side)};
    res += " " + CastStringToCastling(castling);
    res += " " + CastCoordToString(en_passant_coord);
    res += " " + std::to_string(quiet_move_count);
    res += " " + std::to_string((move_count + 1) / 2);
    return res;
}

void Board::MakeBitboards() {
    for (int8_t i = 0; i < NUMBER_OF_CELLS; i++) {
        bb_pieces[i] = 0;
    }
    for (const auto i : {Color::White, Color::Black}) {
        bb_colors[static_cast<int8_t>(i)] = 0;
    }
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        q_util::SetBit(bb_pieces[cells[i]], i);
        if (cells[i] != EMPTY_CELL) {
            q_util::SetBit(bb_colors[static_cast<int8_t>(GetCellColor(cells[i]))], i);
        }
    }
}

void Board::MakeHash() {
    hash = 0;
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        hash ^= MakeZobristHashFromCell(i, cells[i]);
    }
    hash ^= MakeZobristHashFromEnPassantCoord(en_passant_coord);
    hash ^= MakeZobristHashFromCastling(castling);
    hash ^= MakeZobristHashFromMoveSide(move_side);
}

}  // namespace q_core