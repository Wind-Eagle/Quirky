#ifndef QUIRKY_BENCH_CORE_TEST_BOARDS_H
#define QUIRKY_BENCH_CORE_TEST_BOARDS_H

#define CALL_FOR_ALL_TEST_BOARDS(func)                                                          \
    func(Initial, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");                  \
    func(Sicilian, "r1b1k2r/2qnbppp/p2ppn2/1p4B1/3NPPP1/2N2Q2/PPP4P/2KR1B1R w kq - 0 11");      \
    func(Endgame, "8/1pr2p1R/1b1k2p1/pP6/P6P/6P1/5PK1/1B6 w - - 3 43");                         \
    func(Castling, "r3k2r/pppq1ppp/2np1n2/2b1p1B1/2B1P1b1/2NP1N2/PPPQ1PPP/R3K2R w KQkq - 4 8"); \
    func(Promotion, "3b2rk/3P2pp/8/p7/8/2Q5/PP1pppPP/2R4K b - - 0 1");

#endif  // QUIRKY_BENCH_CORE_TEST_BOARDS_H