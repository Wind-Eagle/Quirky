#include "writer.h"

#include <fstream>

#include "../../src/core/board/board.h"
#include "../../src/core/util.h"
#include "core/board/geometry.h"
#include "core/board/types.h"
#include "reader.h"

#include <random>

void WriteBoardsToCSV(const PositionSet& position_set, OutputSources& output_sources) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> rnd(0.0, 1.0);
    for (const auto& position : position_set.positions) {
        const auto& board = position.board;
        std::array<int8_t, q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES * 2> input_features{};
        for (q_core::coord_t i = 0; i < q_core::BOARD_SIZE; i++) {
            if (board.cells[i] != q_core::EMPTY_CELL) {
                if (board.move_side == q_core::Color::White) {
                    input_features[(static_cast<size_t>(board.cells[i]) - 1) * q_core::BOARD_SIZE +
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
        float target = position.target;

        const auto write = [&](std::ofstream& train_out, std::ofstream& test_out){
            std::ofstream& out = rnd(gen) < output_sources.test_ratio ? test_out : train_out;
            out.write(reinterpret_cast<const char*>(packed_bytes.data()), 96);
            out.write(reinterpret_cast<const char*>(&target), sizeof(float));
        };

        const bool write_preliminary = rnd(gen) < output_sources.preliminary_ratio;
        if (write_preliminary) {
            size_t chunk_num = gen() % output_sources.preliminary_chunks_count;
            write(output_sources.preliminary_train_outs[chunk_num], output_sources.preliminary_test_out);
        }
        size_t chunk_num = gen() % output_sources.chunks_count;
        write(output_sources.train_outs[chunk_num], output_sources.test_out);
    }
}
