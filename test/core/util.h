#ifndef QUIRKY_TEST_CORE_UTIL_H
#define QUIRKY_TEST_CORE_UTIL_H

#include "../../src/core/moves/movegen.h"
#include "../../src/core/util.h"

std::string GetSortedMovesList(q_core::MoveList& move_list);

void TestSimpleMovegen(const std::string_view& fen, const std::string_view& ans);

void TestMakeMoveFunctionSimple(const std::string_view& fen, const std::string_view& move_string);

void TestMakeMoveFunction(const std::string_view& fen, const std::string_view& move_string,
                          const std::string_view& move_string_final);

#endif  // QUIRKY_TEST_CORE_UTIL_H
