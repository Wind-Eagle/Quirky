#include <gtest/gtest.h>

#include "../../../src/core/moves/movegen.h"
#include "../../../src/core/util.h"

std::string GetSortedMovesList(q_core::MoveList& move_list) {
    std::vector<std::string> moves;
    for (size_t i = 0; i < move_list.size; i++) {
        moves.push_back(q_core::CastMoveToString(move_list.moves[i]));
    }
    std::sort(moves.begin(), moves.end());
    std::string res;
    for (size_t i = 0; i < moves.size(); i++) {
        if (i > 0) {
            res += " ";
        }
        res += moves[i];
    }
    return res;
}

void TestMovesInPosition(const std::string_view& fen, const std::string_view& ans) {
    q_core::Board board;
    board.MakeFromFEN(fen);
    q_core::MoveList move_list;
    q_core::GenerateAllMoves(board, move_list);
    std::string res = GetSortedMovesList(move_list);
    EXPECT_EQ(res, ans);
}

TEST(CoreMovesPseudolegalMovegen, Startpos) {
    TestMovesInPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                        "a2a3 a2a4 b1a3 b1c3 b2b3 b2b4 c2c3 c2c4 d2d3 d2d4 e2e3 e2e4 f2f3 f2f4 "
                        "g1f3 g1h3 g2g3 g2g4 h2h3 h2h4");
}

TEST(CoreMovesPseudolegalMovegen, Sicilian) {
    TestMovesInPosition("r1b1k2r/2qnbppp/p2ppn2/1p4B1/3NPPP1/2N2Q2/PPP4P/2KR1B1R w kq b6 0 11",
                        "a2a3 a2a4 b2b3 b2b4 c1b1 c1d2 c3a4 c3b1 c3b5 c3d5 c3e2 d1d2 d1d3 d1e1 "
                        "d4b3 d4b5 d4c6 d4e2 d4e6 d4f5 e4e5 f1b5 f1c4 f1d3 f1e2 f1g2 f1h3 f3d3 "
                        "f3e2 f3e3 f3f2 f3g2 f3g3 f3h3 f4f5 g5f6 g5h4 g5h6 h1g1 h2h3 h2h4");
}

TEST(CoreMovesPseudolegalMovegen, Sicilian2) {
    TestMovesInPosition(
        "r1b1k2r/1pqnbppp/p2ppn2/6B1/3NPPP1/2N2Q2/PPP4P/2KR1B1R b kq g3 0 10",
        "a6a5 a8a7 a8b8 b7b5 b7b6 c7a5 c7b6 c7b8 c7c3 c7c4 c7c5 c7c6 c7d8 d6d5 d7b6 d7b8 d7c5 d7e5 "
        "d7f8 e6e5 e7d8 e7f8 e8d8 e8f8 e8g8 f6d5 f6e4 f6g4 f6g8 f6h5 g7g6 h7h5 h7h6 h8f8 h8g8");
}

TEST(CoreMovesPseudolegalMovegen, EnPassant) {
    TestMovesInPosition(
        "r1bqkbnr/ppp1pppp/2n5/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3",
        "a2a3 a2a4 b1a3 b1c3 b2b3 b2b4 c2c3 c2c4 d1e2 d1f3 d1g4 d1h5 d2d3 d2d4 e1e2 e5d6 e5e6 f1a6 f1b5 f1c4 f1d3 f1e2 f2f3 f2f4 g1e2 g1f3 g1h3 g2g3 g2g4 h2h3 h2h4");
}