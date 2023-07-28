#include <gtest/gtest.h>

#include "../../../src/core/moves/movegen.h"
#include "../../../src/core/util.h"

#include "util.h"

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
        "a2a3 a2a4 b1a3 b1c3 b2b3 b2b4 c2c3 c2c4 d1e2 d1f3 d1g4 d1h5 d2d3 d2d4 e1e2 e5d6 e5e6 f1a6 "
        "f1b5 f1c4 f1d3 f1e2 f2f3 f2f4 g1e2 g1f3 g1h3 g2g3 g2g4 h2h3 h2h4");
}

TEST(CoreMovesPseudolegalMovegen, Promotion) {
    TestMovesInPosition(
        "3R4/6pk/7p/8/6P1/5KN1/3p4/1r6 b - - 0 46",
        "b1a1 b1b2 b1b3 b1b4 b1b5 b1b6 b1b7 b1b8 b1c1 b1d1 b1e1 b1f1 b1g1 b1h1 d2d1b d2d1n d2d1q d2d1r g7g5 g7g6 h6h5 h7g6 h7g8 h7h8");
}

TEST(CoreMovesPseudolegalMovegen, Castling1) {
    TestMovesInPosition(
        "r3kb1r/pppq1pp1/2npbn2/4p1Bp/4P2P/3P3N/PPPQBPP1/RN2K2R w KQkq h6 0 8",
        "a2a3 a2a4 b1a3 b1c3 b2b3 b2b4 c2c3 c2c4 d2a5 d2b4 d2c1 d2c3 d2d1 d2e3 d2f4 d3d4 e1d1 e1f1 e1g1 e2d1 e2f1 e2f3 e2g4 e2h5 f2f3 f2f4 g2g3 g2g4 g5e3 g5f4 g5f6 g5h6 h1f1 h1g1 h1h2 h3f4 h3g1");
}

TEST(CoreMovesPseudolegalMovegen, Castling2) {
    TestMovesInPosition(
        "r3kb1r/pppq1pp1/2npbn2/4p1Bp/2P1P2P/3P3N/PP1QBPP1/RN2K2R b KQkq c3 0 8",
        "a7a5 a7a6 a8b8 a8c8 a8d8 b7b5 b7b6 c6a5 c6b4 c6b8 c6d4 c6d8 c6e7 d6d5 d7c8 d7d8 d7e7 e6c4 e6d5 e6f5 e6g4 e6h3 e8c8 e8d8 e8e7 f6d5 f6e4 f6g4 f6g8 f6h7 f8e7 g7g6 h8g8 h8h6 h8h7");
}

TEST(CoreMovesPseudolegalMovegen, Castling3) {
    TestMovesInPosition(
        "r3k2r/pppqbpp1/2npbn2/4p1Bp/2P1P2P/2NP3N/PP1QBPP1/R3K2R b Qk - 6 11",
        "a7a5 a7a6 a8b8 a8c8 a8d8 b7b5 b7b6 c6a5 c6b4 c6b8 c6d4 c6d8 d6d5 d7c8 d7d8 e6c4 e6d5 e6f5 e6g4 e6h3 e7d8 e7f8 e8d8 e8f8 e8g8 f6d5 f6e4 f6g4 f6g8 f6h7 g7g6 h8f8 h8g8 h8h6 h8h7");
}

TEST(CoreMovesPseudolegalMovegen, Castling4) {
    TestMovesInPosition(
        "r4rk1/pppqbpp1/2npbn2/4p1Bp/2P1P2P/2NP3N/PP1QBPP1/R3K2R w Q - 7 12",
        "a1b1 a1c1 a1d1 a2a3 a2a4 b2b3 b2b4 c3a4 c3b1 c3b5 c3d1 c3d5 c4c5 d2c1 d2c2 d2d1 d2e3 d2f4 d3d4 e1c1 e1d1 e1f1 e2d1 e2f1 e2f3 e2g4 e2h5 f2f3 f2f4 g2g3 g2g4 g5e3 g5f4 g5f6 g5h6 h1f1 h1g1 h1h2 h3f4 h3g1");
}