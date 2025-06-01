#include "writer.h"

#include <fstream>

#include "../../src/core/board/board.h"
#include "../../src/core/util.h"
#include "core/board/geometry.h"
#include "core/board/types.h"
#include "eval/score.h"
#include "reader.h"
#include "util/io.h"

float GetTarget(Result result, q_core::Color c) {
    switch (result) {
        case Result::WhiteWins: {
            return c == q_core::Color::White ? 1 : -1;
        }
        case Result::Draw: {
            return 0;
        }
        case Result::BlackWins: {
            return c == q_core::Color::White ? -1 : 1;
        }
        default: {
            q_util::ExitWithError(QuirkyError::UnexpectedValue);
        }
    }
    return 0;
}

void WriteBoardsToCSV(const GameSet& game_set, std::ofstream& out) {
    for (const auto& game : game_set.games) {
        for (const auto& board : game.boards) {
            std::array<int8_t, q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2> input_features{};
            for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
                if (board.cells[i] != q_core::EMPTY_CELL) {
                    if (board.move_side == q_core::Color::White) {
                        input_features[(static_cast<size_t>(board.cells[i]) - 1) *
                                           q_core::BOARD_SIZE +
                                       i]++;
                    } else {
                        input_features[(static_cast<size_t>(q_core::FlipCellColor(board.cells[i])) -
                                        1) *
                                           q_core::BOARD_SIZE +
                                       q_core::FlipCoord(i)]++;
                    }
                }
            }
            std::vector<uint8_t> packed_bytes(96, 0);
            for (size_t byte_idx = 0; byte_idx < 96; ++byte_idx) {
                for (int bit_idx = 0; bit_idx < 8; ++bit_idx) {
                    size_t global_bit_idx = byte_idx * 8 + bit_idx;
                    if (input_features[global_bit_idx]) {
                        packed_bytes[byte_idx] |= (1 << (7 - bit_idx));
                    }
                }
            }
            float target = GetTarget(game.header.result, board.move_side);

            out.write(reinterpret_cast<const char*>(packed_bytes.data()), 96);
            out.write(reinterpret_cast<const char*>(&target), sizeof(float));
        }
    }
}
