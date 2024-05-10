#include "util.h"

#include <gtest/gtest.h>

#include <vector>

#include "../../src/core/moves/board_manipulation.h"
#include "../../src/util/string.h"

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

void TestSimpleMovegen(const std::string_view& fen, const std::string_view& ans) {
    q_core::Board board;
    board.MakeFromFEN(fen);
    q_core::MoveList move_list;
    q_core::GenerateAllMoves(board, move_list);
    std::string res = GetSortedMovesList(move_list);
    EXPECT_EQ(res, ans);
}

void TestMakeMoveFunctionSimple(const std::string_view& fen, const std::string_view& move_string) {
    std::vector<std::string> parsed_move_string = q_util::SplitString(move_string);
    std::vector<q_core::Move> moves(parsed_move_string.size());
    q_core::Board board;
    board.MakeFromFEN(fen);
    for (size_t i = 0; i < parsed_move_string.size(); i++) {
        moves[i] = q_core::TranslateStringToMove(board, parsed_move_string[i]);
    }
    for (size_t i = 0; i < moves.size(); i++) {
        q_core::MakeMoveInfo info;
        q_core::MakeMove(board, moves[i], info);
        EXPECT_TRUE(board.IsValid());
        if (q_core::WasMoveLegal(board, moves[i])) {
            EXPECT_TRUE(board.IsValid());
        }
        q_core::UnmakeMove(board, moves[i], info);
        EXPECT_EQ(board.GetFEN(), fen);
    }
}

void TestMakeMoveFunction(const std::string_view& fen, const std::string_view& move_string, const std::string_view& move_string_final) {
    std::vector<std::string> parsed_move_string = q_util::SplitString(move_string);
    std::vector<q_core::Move> moves(parsed_move_string.size());
    q_core::Board board;
    board.MakeFromFEN(fen);
    for (size_t i = 0; i < parsed_move_string.size(); i++) {
        moves[i] = q_core::TranslateStringToMove(board, parsed_move_string[i]);
        q_core::MakeMoveInfo info;
        q_core::MakeMove(board, moves[i], info);
        EXPECT_TRUE(q_core::WasMoveLegal(board, moves[i]));
        EXPECT_TRUE(board.IsValid());
    }
    TestSimpleMovegen(board.GetFEN(), move_string_final);
}
