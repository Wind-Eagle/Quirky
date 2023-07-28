#ifndef QUIRKY_TEST_CORE_MOVES_FUNC_H
#define QUIRKY_TEST_CORE_MOVES_FUNC_H

#include "../../../src/core/moves/movegen.h"
#include "../../../src/core/util.h"

std::string GetSortedMovesList(q_core::MoveList& move_list);

void TestMovesInPosition(const std::string_view& fen, const std::string_view& ans);

void TestMakeMoveFunctionSimple(const std::string_view& fen, const std::string_view& move_string);

#endif  // QUIRKY_TEST_CORE_MOVES_FUNC_H
