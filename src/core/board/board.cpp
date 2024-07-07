#include "board.h"

#include <cstring>
#include <vector>

#include "../../util/bit.h"
#include "../../util/string.h"
#include "../util.h"
#include "geometry.h"
#include "hash.h"
#include "types.h"

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
            Q_UNREACHABLE();
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
        res |= Castling::BlackKingside;
    }
    if (str.find('K') != std::string::npos) {
        res |= Castling::WhiteKingside;
    }
    if (str.find('q') != std::string::npos) {
        res |= Castling::BlackQueenside;
    }
    if (str.find('Q') != std::string::npos) {
        res |= Castling::WhiteQueenside;
    }
    return res;
}

inline std::string CastCastlingToString(const Castling c) {
    Q_ASSERT(IsCastlingValid(c));
    std::string res;
    if (IsCastlingAllowed(c, Castling::WhiteKingside)) {
        res += "K";
    }
    if (IsCastlingAllowed(c, Castling::WhiteQueenside)) {
        res += "Q";
    }
    if (IsCastlingAllowed(c, Castling::BlackKingside)) {
        res += "k";
    }
    if (IsCastlingAllowed(c, Castling::BlackQueenside)) {
        res += "q";
    }
    if (res.empty()) {
        res = "-";
    }
    return res;
}

inline bool IsMoveSideStringValid(const std::string& str) { return str == "w" || str == "b"; }

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

constexpr subcoord_t WHITE_EN_PASSANT_RANK = 5;
constexpr subcoord_t BLACK_EN_PASSANT_RANK = 2;

inline bool IsEnPassantCoordValid(const Color c, const coord_t coord) {
    if (coord == NO_ENPASSANT_COORD) {
        return true;
    }
    if (!IsCoordValid(coord)) {
        return false;
    }
    return GetRank(coord) == (c == Color::White ? WHITE_EN_PASSANT_RANK : BLACK_EN_PASSANT_RANK);
}

inline bool IsMoveCountStringValid(const std::string& str) {
    for (const auto i : str) {
        if (!std::isdigit(i)) {
            return false;
        }
    }
    return true;
}

inline std::string CastEnPassantCoordToString(const coord_t c) {
    Q_ASSERT(IsCoordValid(c));
    if (c == NO_ENPASSANT_COORD) {
        return "-";
    }
    return std::string({static_cast<char>(GetFile(c) + 'a'), static_cast<char>(GetRank(c) + '1')});
}

inline constexpr coord_t CastStringToEnPassantCoord(const std::string_view& str) {
    if (str == "-") {
        return NO_ENPASSANT_COORD;
    }
    Q_ASSERT(IsCoordStringValid(str));
    return ((str[1] - '1') << BOARD_SIDE_LOG) + str[0] - 'a';
}

#define Q_CHECK_FEN_PARSE_ERROR(condition, error) \
    do {                                          \
        if (!(condition)) {                       \
            return error;                         \
        }                                         \
    } while (false)

Board::FENParseStatus Board::MakeFromFEN(const std::string_view& fen) {
    std::vector<std::string> parsed_fen = q_util::SplitString(fen);
    Q_CHECK_FEN_PARSE_ERROR(parsed_fen.size() == 6, FENParseStatus::InvalidNumberOfFENFields);
    std::vector<std::string> parsed_position = q_util::SplitString(parsed_fen[0], '/');
    Q_CHECK_FEN_PARSE_ERROR(parsed_position.size() == BOARD_SIDE,
                            FENParseStatus::InvalidNumberOfRows);
    for (subcoord_t rank = 0; rank < BOARD_SIDE; rank++) {
        subcoord_t file = 0;
        for (auto c : parsed_position[rank]) {
            Q_CHECK_FEN_PARSE_ERROR(file < BOARD_SIDE, FENParseStatus::InvalidNumberOfColumns);
            if (isdigit(c)) {
                Q_CHECK_FEN_PARSE_ERROR((c - '0') > 0 && (c - '0') <= BOARD_SIDE,
                                        FENParseStatus::InvalidSizeOfColumnSkip);
                for (coord_t cur = file; cur < file + c - '0'; cur++) {
                    cells[MakeCoord(InvertSubcoord(rank), cur)] = EMPTY_CELL;
                }
                file += c - '0';
            } else {
                cell_t cell = CastCharToCell(c);
                Q_CHECK_FEN_PARSE_ERROR(cell != UNDEFINED_CELL, FENParseStatus::InvalidCell);
                cells[MakeCoord(InvertSubcoord(rank), file)] = cell;
                file++;
            }
            Q_CHECK_FEN_PARSE_ERROR(file <= BOARD_SIDE, FENParseStatus::InvalidNumberOfColumns);
        }
        Q_CHECK_FEN_PARSE_ERROR(file == BOARD_SIDE, FENParseStatus::InvalidNumberOfColumns);
    }
    Q_CHECK_FEN_PARSE_ERROR(IsMoveSideStringValid(parsed_fen[1]), FENParseStatus::InvalidMoveSide);
    move_side = CastCharToColor(parsed_fen[1][0]);
    Q_CHECK_FEN_PARSE_ERROR(IsCastlingStringValid(parsed_fen[2]), FENParseStatus::InvalidCastling);
    castling = CastStringToCastling(parsed_fen[2]);
    Q_CHECK_FEN_PARSE_ERROR(
        IsEnPassantCoordValid(move_side, CastStringToEnPassantCoord(parsed_fen[3])),
        FENParseStatus::InvalidEnPassantCoord);
    en_passant_coord = CastStringToEnPassantCoord(parsed_fen[3]);
    Q_CHECK_FEN_PARSE_ERROR(IsMoveCountStringValid(parsed_fen[4]),
                            FENParseStatus::InvalidQuietMoveCount);
    fifty_rule_move_count = stoi(parsed_fen[4]);
    Q_CHECK_FEN_PARSE_ERROR(IsMoveCountStringValid(parsed_fen[5]),
                            FENParseStatus::InvalidMoveCount);
    move_count = stoi(parsed_fen[5]) * 2 - (move_side == Color::White ? 1 : 0);
    MakeBitboards();
    MakeHash();
    Q_ASSERT(IsValid());
    return FENParseStatus::Ok;
}

std::string Board::GetFEN() const {
    Q_ASSERT(IsValid());
    std::string res;
    for (subcoord_t rank = 0; rank < BOARD_SIDE; rank++) {
        uint8_t block_size = 0;
        for (subcoord_t file = 0; file < BOARD_SIDE; file++) {
            coord_t cur = MakeCoord(InvertSubcoord(rank), file);
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
        if (rank != BOARD_SIDE - 1) {
            res += "/";
        }
    }
    res += {' ', CastColorToChar(move_side)};
    res += " " + CastCastlingToString(castling);
    res += " " + CastEnPassantCoordToString(en_passant_coord);
    res += " " + std::to_string(static_cast<int>(fifty_rule_move_count));
    res += " " + std::to_string((move_count + 1) / 2);
    return res;
}

bool Board::IsValid() const {
    for (coord_t i = 0; i < BOARD_SIZE; i++) {
        if (!IsCellValid(cells[i])) {
            return false;
        }
    }
    if (!(IsCastlingValid(castling) && IsColorValid(move_side) &&
          IsEnPassantCoordValid(move_side, en_passant_coord))) {
        return false;
    }
    if (q_util::GetBitCount(bb_colors[0]) > 16 || q_util::GetBitCount(bb_colors[1]) > 16) {
        return false;
    }
    if (q_util::GetBitCount(bb_pieces[MakeCell(Color::White, Piece::King)]) != 1 ||
        q_util::GetBitCount(bb_pieces[MakeCell(Color::Black, Piece::King)]) != 1) {
        return false;
    }
    if ((bb_pieces[MakeCell(Color::White, Piece::Pawn)] |
         bb_pieces[MakeCell(Color::Black, Piece::Pawn)]) &
        (RANK_BITBOARD[0] | RANK_BITBOARD[BOARD_SIDE - 1])) {
        return false;
    }
    Board old_board = (*this);
    old_board.MakeBitboards();
    old_board.MakeHash();
    return memcmp(static_cast<void*>(&old_board), this, sizeof(Board)) == 0;
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