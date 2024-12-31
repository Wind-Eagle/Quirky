#include "writer.h"

#include <fstream>

#include "reader.h"

#include "../../src/core/board/board.h"
#include "../../src/core/util.h"
#include "../../src/eval/evaluator.h"
#include "../../src/eval/model.h"

std::string GetTarget(Result result) {
    switch (result) {
        case Result::WhiteWins: {
            return "1";
        }
        case Result::Draw: {
            return "0";
        }
        case Result::BlackWins: {
            return "-1";
        }
        default: {
            q_util::ExitWithError(QuirkyError::UnexpectedValue);
        }
    }
    return "0";
}

void WriteBoardsToCSV(const GameSet& game_set, std::ofstream& out) {
    out << "stage,move_count,weight";
    for (size_t i = 0; i < q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2; i++) {
        out << "," << i;
    }
    out << ",target\n";
    for (const auto& game: game_set.games) {
        for (const auto& board: game.boards) {
            std::array<int8_t, q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2> input_features{};
            q_eval::stage_t stage = 0;
            for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
                if (board.cells[i] != q_core::EMPTY_CELL) {
                    input_features[(static_cast<size_t>(board.cells[i]) - 1) * q_core::BOARD_SIZE + i]++;
                }
            }
            // Stage is unused
            stage = 0;
            out << static_cast<int>(stage) << "," << game.boards.size() << ",1";
            for (size_t i = 0; i < q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2; i++) {
                out << "," << static_cast<int>(input_features[i]);
            }
            out << "," << GetTarget(game.header.result);
            out << '\n';
        }
    }
}
