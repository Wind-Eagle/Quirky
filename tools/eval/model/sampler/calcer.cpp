#include "calcer.h"

uint16_t GetInnerFeatureStoragePSQIndex(const q_core::coord_t coord, const q_core::Piece piece) {
    return (static_cast<uint16_t>(piece) - 1) * q_core::BOARD_SIZE + coord;
}

void AddToInnerFeaturePSQStorage(
    std::array<int8_t, q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES>& storage,
    const uint16_t index, int8_t value) {
    Q_ASSERT(index >= q_eval::FEATURE_COUNT);
    const uint16_t psq_index = index - q_eval::FEATURE_COUNT;
    const q_core::coord_t coord = q_eval::GetCoordFromPSQIndex(psq_index);
    const q_core::cell_t cell = q_eval::GetCellFromPSQIndex(psq_index);
    if (cell == q_core::EMPTY_CELL) {
        return;
    }
    const q_core::Piece piece = q_core::GetCellPiece(cell);
    if (q_core::GetCellColor(cell) == q_core::Color::Black) {
        value *= -1;
    }
    const uint16_t final_index = GetInnerFeatureStoragePSQIndex(coord, piece);
    storage[final_index] += value;
}

void AddToInnerFeatureStorage(std::array<int8_t, q_eval::FEATURE_COUNT>& storage,
                              const uint16_t index, int8_t value) {
    Q_ASSERT(index < q_eval::FEATURE_COUNT);
    storage[index] += value;
}

BoardSetWithFeatures CalcFeatures(GameSet&& game_set_ref) {
    const GameSet game_set = std::move(game_set_ref);
    BoardSetWithFeatures res;
    res.file_path = game_set.file_path;
    for (size_t i = 0; i < game_set.games.size(); i++) {
        const auto& game = game_set.games[i];
        const auto& header = game.header;
        for (const auto& board : game.boards) {
            q_eval::Evaluator<q_eval::EvaluationType::Vector> evaluator;
            evaluator.StartTrackingBoard(board);
            const auto& features = evaluator.Evaluate(board).GetFeatures();
            std::array<int8_t, q_eval::FEATURE_COUNT> feature_storage = {};
            std::array<int8_t, q_core::BOARD_SIZE * q_core::NUMBER_OF_PIECES> psq_storage = {};
            for (size_t j = 0; j < features.size(); j++) {
                if (j < q_eval::FEATURE_COUNT) {
                    AddToInnerFeatureStorage(feature_storage, j, features[j]);
                } else {
                    AddToInnerFeaturePSQStorage(psq_storage, j, features[j]);
                }
            }
            FeatureStorageWithInfo feature_storage_with_info = {.feature_storage = feature_storage,
                                                                .psq_storage = psq_storage,
                                                                .stage = evaluator.GetStage(),
                                                                .game_index = i};
            BoardWithFeatures board_with_features = {
                .board = board, .feature_storage_with_info = feature_storage_with_info};
            res.boards.push_back(board_with_features);
        }
    }
    return res;
}
