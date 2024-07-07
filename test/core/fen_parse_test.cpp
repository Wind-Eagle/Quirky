#include <gtest/gtest.h>

#include "../../src/core/board/board.h"
#include "../../src/core/util.h"

template <class T>
void CompareArrays(const T* lhs, const T* rhs, size_t size) {
    for (size_t i = 0; i < size; i++) {
        EXPECT_EQ(lhs[i], rhs[i]) << std::to_string(i) << "th elements are not equal" << std::endl;
    }
}

void TestStartpos() {
    q_core::Board board;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    q_core::Board::FENParseStatus parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::Ok);
    std::array<uint64_t, 13> expected_bb_pieces = {281474976645120ULL,
                                                   65280ULL,
                                                   66ULL,
                                                   36ULL,
                                                   129ULL,
                                                   8ULL,
                                                   16ULL,
                                                   71776119061217280ULL,
                                                   4755801206503243776ULL,
                                                   2594073385365405696ULL,
                                                   9295429630892703744ULL,
                                                   576460752303423488ULL,
                                                   1152921504606846976ULL};
    std::array<uint64_t, 2> expected_bb_colors = {65535ULL, 18446462598732840960ULL};
    std::array<int8_t, 64> expected_cells = {4, 2, 3, 5, 6, 3, 2, 4, 1,  1, 1, 1,  1,  1, 1, 1,
                                             0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0,
                                             7, 7, 7, 7, 7, 7, 7, 7, 10, 8, 9, 11, 12, 9, 8, 10};
    CompareArrays<q_core::bitboard_t>(board.bb_pieces, expected_bb_pieces.data(),
                                      expected_bb_pieces.size());
    CompareArrays<q_core::bitboard_t>(board.bb_colors, expected_bb_colors.data(),
                                      expected_bb_colors.size());
    CompareArrays<q_core::cell_t>(board.cells, expected_cells.data(), 64);
    EXPECT_NE(board.hash, 0);
    EXPECT_EQ(board.en_passant_coord, q_core::NO_ENPASSANT_COORD);
    EXPECT_EQ(board.castling, q_core::Castling::All);
    EXPECT_EQ(board.move_side, q_core::Color::White);
    EXPECT_EQ(board.fifty_rule_move_count, 0);
    EXPECT_EQ(board.move_count, 1);
    EXPECT_EQ(board.GetFEN(), fen);
}

void TestTrickyPos() {
    q_core::Board board;
    std::string fen = "r1bq3r/n1pk1ppp/p2p1n2/2bPp3/Pp2P3/1BP2N1P/1P3PP1/RNBQ1RK1 b - a3 0 11";
    q_core::Board::FENParseStatus parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::Ok);
    std::array<uint64_t, 13> expected_bb_pieces = {8219868544315792784ULL,
                                                   34653626880ULL,
                                                   2097154ULL,
                                                   131076ULL,
                                                   33ULL,
                                                   8ULL,
                                                   64ULL,
                                                   64186259047710720ULL,
                                                   316659348799488ULL,
                                                   288230393331580928ULL,
                                                   9295429630892703744ULL,
                                                   576460752303423488ULL,
                                                   2251799813685248ULL};
    std::array<uint64_t, 2> expected_bb_colors = {34655855215ULL, 10226875494737903616ULL};
    std::array<int8_t, 64> expected_cells = {4, 2, 3, 5,  0, 4, 6, 0, 0,  1, 0, 0,  0, 1, 1, 0,
                                             0, 3, 1, 0,  0, 2, 0, 1, 1,  7, 0, 0,  1, 0, 0, 0,
                                             0, 0, 9, 1,  7, 0, 0, 0, 7,  0, 0, 7,  0, 8, 0, 0,
                                             8, 0, 7, 12, 0, 7, 7, 7, 10, 0, 9, 11, 0, 0, 0, 10};
    CompareArrays<q_core::bitboard_t>(board.bb_pieces, expected_bb_pieces.data(),
                                      expected_bb_pieces.size());
    CompareArrays<q_core::bitboard_t>(board.bb_colors, expected_bb_colors.data(),
                                      expected_bb_colors.size());
    CompareArrays<q_core::cell_t>(board.cells, expected_cells.data(), 64);
    EXPECT_NE(board.hash, 0);
    EXPECT_EQ(board.en_passant_coord, q_core::CastStringToCoord("a3"));
    EXPECT_EQ(board.move_side, q_core::Color::Black);
    EXPECT_EQ(board.fifty_rule_move_count, 0);
    EXPECT_EQ(board.move_count, 22);
    EXPECT_EQ(board.GetFEN(), fen);
}

void TestMoveCount() {
    q_core::Board board;
    std::string fen = "8/1R1K4/6r1/3k4/5b2/8/8/8 b - - 99 171";
    q_core::Board::FENParseStatus parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::Ok);
    EXPECT_EQ(board.fifty_rule_move_count, 99);
    EXPECT_EQ(board.move_count, 342);
    EXPECT_EQ(board.castling, q_core::Castling::None);
    fen = "2K5/1R6/6r1/2k5/8/8/3b4/8 w - - 94 169";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::Ok);
    EXPECT_EQ(board.fifty_rule_move_count, 94);
    EXPECT_EQ(board.move_count, 337);
}

void TestErrors() {
    q_core::Board board;
    std::string fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 p w kq - 2 7";
    q_core::Board::FENParseStatus parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidNumberOfFENFields);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 kq - 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidNumberOfFENFields);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1/8 w kq - 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidNumberOfRows);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ2RK1 w kq - 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidNumberOfColumns);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPPPPP/RNBQ1RK1 w kq - 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidNumberOfColumns);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P03/1B3N2/PPPP1PPP/RNBQ1RK1 w kq - 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidSizeOfColumnSkip);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P93/1B3N2/PPPP1PPP/RNBQ1RK1 w kq - 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_TRUE(parse_status == q_core::Board::FENParseStatus::InvalidSizeOfColumnSkip ||
                parse_status == q_core::Board::FENParseStatus::InvalidNumberOfColumns);
    fen = "r1wqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 w kq - 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidCell);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 q kq - 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidMoveSide);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 w wq - 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidCastling);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 w kq a2 2 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidEnPassantCoord);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 w kq - -1 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidQuietMoveCount);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 w kq - abc 7";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidQuietMoveCount);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 w kq - 2 -1";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidMoveCount);
    fen = "r1bqk2r/2pp1ppp/p1n2n2/1pb1p3/4P3/1B3N2/PPPP1PPP/RNBQ1RK1 w kq - 2 abc";
    parse_status = board.MakeFromFEN(fen);
    EXPECT_EQ(parse_status, q_core::Board::FENParseStatus::InvalidMoveCount);
}

TEST(CoreMakeFromFEN, Startpos) { TestStartpos(); }

TEST(CoreMakeFromFEN, TrickyPos) { TestTrickyPos(); }

TEST(CoreMakeFromFEN, MoveCount) { TestMoveCount(); }

TEST(CoreMakeFromFEN, Errors) { TestErrors(); }

TEST(CoreMakeFromFEN, Rewrite) {
    TestStartpos();
    TestErrors();
    TestStartpos();
    TestTrickyPos();
    TestTrickyPos();
    TestErrors();
    TestTrickyPos();
    TestMoveCount();
    TestTrickyPos();
    TestTrickyPos();
    TestStartpos();
    TestTrickyPos();
    TestStartpos();
    TestMoveCount();
    TestStartpos();
    TestErrors();
    TestMoveCount();
    TestErrors();
}
