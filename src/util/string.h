#ifndef QUIRKY_SRC_UTIL_STRING_H
#define QUIRKY_SRC_UTIL_STRING_H

#include <cstring>
#include <string>
#include <vector>

#include "../types/types_operations.h"

namespace util {

/// Calculates a simple hash from a C-style string in a compile time. To make it harder, one iteration of
/// Xorshift* 64 is performed on this hash.
inline constexpr uint64_t GetStringHash(const char* str) {
    size_t n = sizeof(str);
    uint64_t ans = 0;
    for (size_t i = 0; i < n; i++) {
        ans = ans * 100003 + str[i];
    }
    ans ^= ans >> 12;
    ans ^= ans << 25;
    ans ^= ans >> 27;
    return ans * 0x2545F4914F6CDD1DULL;
}

/// Splits a string by a delimiter. Multiple delimiters are not handled as one.
inline std::vector<std::string> SplitString(const std::string& str, const char delim = ' ') {
    std::vector<std::string> res;
    size_t start = 0;
    size_t end;
    while ((end = str.find(delim, start)) != std::string::npos) {
        res.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    res.push_back(str.substr(start, end - start));
    return res;
}

/// Makes char from cell_t. For convenience '.' symbol is an empty cell, and invalid cell is a
/// symbol that doesn't match anything, though it is not used.
inline constexpr core::cell_t CharToCell(const char c) {
    switch (c) {
        case 'P':
            return MakeCell(core::Color::White, core::Piece::Pawn);
        case 'N':
            return MakeCell(core::Color::White, core::Piece::Knight);
        case 'B':
            return MakeCell(core::Color::White, core::Piece::Bishop);
        case 'R':
            return MakeCell(core::Color::White, core::Piece::Rook);
        case 'Q':
            return MakeCell(core::Color::White, core::Piece::Queen);
        case 'K':
            return MakeCell(core::Color::White, core::Piece::King);
        case 'p':
            return MakeCell(core::Color::Black, core::Piece::Pawn);
        case 'n':
            return MakeCell(core::Color::Black, core::Piece::Knight);
        case 'b':
            return MakeCell(core::Color::Black, core::Piece::Bishop);
        case 'r':
            return MakeCell(core::Color::Black, core::Piece::Rook);
        case 'q':
            return MakeCell(core::Color::Black, core::Piece::Queen);
        case 'k':
            return MakeCell(core::Color::Black, core::Piece::King);
        case '.':
            return core::EMPTY_CELL;
        default:
            return core::INVALID_CELL;
    }
}

/// Makes cell_t from char. For convenience empty cell is a '.'symbol and invalid cell is a '?'
/// symbol, though it is not used.
inline constexpr char CellToChar(const core::cell_t c) {
    switch (c) {
        case MakeCell(core::Color::White, core::Piece::Pawn):
            return 'P';
        case MakeCell(core::Color::White, core::Piece::Knight):
            return 'N';
        case MakeCell(core::Color::White, core::Piece::Bishop):
            return 'B';
        case MakeCell(core::Color::White, core::Piece::Rook):
            return 'R';
        case MakeCell(core::Color::White, core::Piece::Queen):
            return 'Q';
        case MakeCell(core::Color::White, core::Piece::King):
            return 'K';
        case MakeCell(core::Color::Black, core::Piece::Pawn):
            return 'p';
        case MakeCell(core::Color::Black, core::Piece::Knight):
            return 'n';
        case MakeCell(core::Color::Black, core::Piece::Bishop):
            return 'b';
        case MakeCell(core::Color::Black, core::Piece::Rook):
            return 'r';
        case MakeCell(core::Color::Black, core::Piece::Queen):
            return 'q';
        case MakeCell(core::Color::Black, core::Piece::King):
            return 'k';
        case core::EMPTY_CELL:
            return '.';
        default:
            return '?';
    }
}

inline constexpr core::Color CharToColor(const char c) {
    return c == 'w' ? core::Color::White : core::Color::Black;
}

inline constexpr char ColorToChar(const core::Color c) {
    return c == core::Color::White ? 'w' : 'b';
}

inline core::Castling StringToCastling(const std::string& str) {
    core::Castling res = core::Castling::None;
    if (str.find('k') != std::string::npos) {
        res |= core::Castling::BlackKingSide;
    }
    if (str.find('K') != std::string::npos) {
        res |= core::Castling::WhiteKingSide;
    }
    if (str.find('q') != std::string::npos) {
        res |= core::Castling::BlackQueenSide;
    }
    if (str.find('Q') != std::string::npos) {
        res |= core::Castling::WhiteQueenSide;
    }
    return res;
}

inline std::string StringToCastling(const core::Castling cst) {
    std::string res;
    if (util::IsAnyCastling(cst & core::Castling::WhiteKingSide)) {
        res += "K";
    }
    if (util::IsAnyCastling(cst & core::Castling::WhiteQueenSide)) {
        res += "Q";
    }
    if (util::IsAnyCastling(cst & core::Castling::BlackKingSide)) {
        res += "k";
    }
    if (util::IsAnyCastling(cst & core::Castling::BlackQueenSide)) {
        res += "q";
    }
    return res;
}

inline core::coord_t StringToCoord(const std::string& str) {
    if (str == "-") {
        return core::INVALID_COORD;
    }
    return ((str[1] - '0') << 3) + str[0] - 'a';
}

inline std::string CoordToString(const core::coord_t coord) {
    if (coord == core::INVALID_COORD) {
        return "-";
    }
    return std::string(
        {static_cast<char>((coord & 7) + 'a'), static_cast<char>((coord >> 3) + 'a')});
}

}  // namespace util

#endif  // QUIRKY_SRC_UTIL_STRING_H
