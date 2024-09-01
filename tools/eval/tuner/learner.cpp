#include "learner.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <queue>

#include "../../../src/eval/psq.h"
#include "../../../src/util/processor.h"
#include "calcer.h"

// static constexpr size_t BATCH_SIZE = 1024;

void AssignWeights(const std::array<int16_t, PSQ_AND_FEATURE_COUNT>& weights) {
    for (size_t i = 0; i < q_core::NUMBER_OF_PIECES; i++) {
        q_eval::PIECE_COST[i] = q_eval::ScorePair(weights[i], weights[i + TAPERED_EVAL_BOUND]);
    }
    std::array<q_eval::ScorePair, COMPRESSED_PSQ_SIZE> raw_psq;
    for (size_t i = 0; i < COMPRESSED_PSQ_SIZE; i++) {
        raw_psq[i] = q_eval::ScorePair(weights[i + q_core::NUMBER_OF_PIECES],
                                       weights[i + q_core::NUMBER_OF_PIECES + TAPERED_EVAL_BOUND]);
    }
    q_eval::PSQ = GetPSQ(raw_psq);
    for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
        q_eval::MODEL_WEIGHTS[i] = q_eval::ScorePair(
            weights[i + q_core::NUMBER_OF_PIECES + COMPRESSED_PSQ_SIZE],
            weights[i + q_core::NUMBER_OF_PIECES + COMPRESSED_PSQ_SIZE + TAPERED_EVAL_BOUND]);
    }
}

double GetLossByWeights(const LearnerParams& learner_params,
                        const std::vector<std::shared_ptr<Element>>& elements,
                        const std::array<int16_t, PSQ_AND_FEATURE_COUNT>& weights,
                        CalcerParams calcer_params) {
    AssignWeights(weights);
    q_util::Processor<std::shared_ptr<Element>, double> scoring_processor(
        learner_params.number_of_threads, learner_params.channel_size, learner_params.channel_size);
    scoring_processor.Start(
        [&](std::shared_ptr<Element> game) { return GetCalcerResult(game, calcer_params); });
    std::thread sender([&]() {
        for (size_t i = 0; i < elements.size(); i++) {
            scoring_processor.Send(elements[i]);
        }
        scoring_processor.Stop();
    });
    double total_loss = 0;
    size_t number_of_positions = 0;
    for (;;) {
        std::optional<double> calcer_result = scoring_processor.Receive();
        if (calcer_result != std::nullopt) {
            total_loss += *calcer_result;
            number_of_positions++;
        } else {
            break;
        }
    }
    sender.join();
    return total_loss / number_of_positions;
}

void WriteResultToFile(const std::array<int16_t, PSQ_AND_FEATURE_COUNT>& weights,
                       const std::string& output_filename) {
    std::ofstream out(output_filename);
    out << "pieces cost:" << std::endl;
    for (size_t i = 0; i < q_core::NUMBER_OF_PIECES; i++) {
        if (i > 0) {
            out << ", ";
        }
        out << "ScorePair(" << weights[i] << ", " << weights[i + TAPERED_EVAL_BOUND] << ")";
    }
    out << std::endl;
    out << "psq:" << std::endl;
    for (size_t i = 0; i < COMPRESSED_PSQ_SIZE; i++) {
        if (i > 0) {
            out << ", ";
        }
        out << "ScorePair(" << weights[i + q_core::NUMBER_OF_PIECES] << ", "
            << weights[i + q_core::NUMBER_OF_PIECES + TAPERED_EVAL_BOUND] << ")";
    }
    out << std::endl;
    out << "model weights:" << std::endl;
    for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
        if (i > 0) {
            out << ", ";
        }
        out << "ScorePair(" << weights[i + q_core::NUMBER_OF_PIECES + COMPRESSED_PSQ_SIZE] << ", "
            << weights[i + q_core::NUMBER_OF_PIECES + COMPRESSED_PSQ_SIZE + TAPERED_EVAL_BOUND]
            << ")";
    }
    out << std::endl;
}

bool ReadStateFromFile(std::array<int16_t, PSQ_AND_FEATURE_COUNT>& weights,
                       std::array<double, PSQ_AND_FEATURE_COUNT>& grad_preference,
                       std::array<double, PSQ_AND_FEATURE_COUNT>& improvements,
                       const std::string& state_filename) {
    if (!std::filesystem::exists(state_filename)) {
        return false;
    }
    std::ifstream in(state_filename);
    if (!in.is_open()) {
        return false;
    }
    for (size_t i = 0; i < PSQ_AND_FEATURE_COUNT; i++) {
        int16_t weight;
        double grad;
        double impr;
        if (in.eof() || in.fail() || in.bad()) {
            return false;
        }
        in >> weight >> grad >> impr;
        weights[i] = weight;
        grad_preference[i] = grad;
        improvements[i] = impr;
    }
    return true;
}

void WriteStateToFile(const std::array<int16_t, PSQ_AND_FEATURE_COUNT>& weights,
                      const std::array<double, PSQ_AND_FEATURE_COUNT>& grad_preference,
                      const std::array<double, PSQ_AND_FEATURE_COUNT>& improvements,
                      const std::string& state_filename) {
    std::ofstream out(state_filename);
    for (size_t i = 0; i < PSQ_AND_FEATURE_COUNT; i++) {
        out << std::fixed << std::setprecision(12) << weights[i] << " " << grad_preference[i] << " "
            << improvements[i] << std::endl;
    }
}

struct FeatureWithImprovement {
    double improvement;
    size_t num;
};

bool operator<(FeatureWithImprovement a, FeatureWithImprovement b) {
    return a.improvement < b.improvement;
}

void GetDefaultState(std::array<int16_t, PSQ_AND_FEATURE_COUNT>& weights,
                     std::array<double, PSQ_AND_FEATURE_COUNT>& grad_preference,
                     std::array<double, PSQ_AND_FEATURE_COUNT>& improvements) {
    for (size_t i = 0; i < q_core::NUMBER_OF_PIECES; i++) {
        weights[i] = q_eval::PIECE_COST[i].GetFirst();
        weights[i + TAPERED_EVAL_BOUND] = q_eval::PIECE_COST[i].GetSecond();
    }
    for (size_t i = 0; i < COMPRESSED_PSQ_SIZE; i++) {
        const q_core::coord_t coord = static_cast<q_core::coord_t>(i % q_core::BOARD_SIZE);
        const q_core::cell_t cell = q_core::MakeCell(
            q_core::Color::White, static_cast<q_core::Piece>(i / q_core::BOARD_SIZE + 1));
        const size_t psq_index = q_eval::GetPSQIndex(cell, coord);
        weights[i + q_core::NUMBER_OF_PIECES] =
            q_eval::PSQ[psq_index].GetFirst() -
            q_eval::PIECE_COST[i / q_core::BOARD_SIZE].GetFirst();
        weights[i + q_core::NUMBER_OF_PIECES + TAPERED_EVAL_BOUND] =
            q_eval::PSQ[psq_index].GetSecond() -
            q_eval::PIECE_COST[i / q_core::BOARD_SIZE].GetSecond();
    }
    for (size_t i = 0; i < q_eval::FEATURE_COUNT; i++) {
        weights[i + q_core::NUMBER_OF_PIECES + COMPRESSED_PSQ_SIZE] =
            q_eval::MODEL_WEIGHTS[i].GetFirst();
        weights[i + q_core::NUMBER_OF_PIECES + COMPRESSED_PSQ_SIZE + TAPERED_EVAL_BOUND] =
            q_eval::MODEL_WEIGHTS[i].GetSecond();
    }
    for (size_t i = 0; i < PSQ_AND_FEATURE_COUNT; i++) {
        grad_preference[i] = 0;
        bool is_piece_weight = i < q_core::NUMBER_OF_PIECES;
        if (i >= TAPERED_EVAL_BOUND && i - TAPERED_EVAL_BOUND < q_core::NUMBER_OF_PIECES) {
            is_piece_weight = true;
        }
        improvements[i] = is_piece_weight ? 1e-3 : 1e-5;
    }
}

struct Stat {
    std::chrono::time_point<std::chrono::steady_clock> time = std::chrono::steady_clock::now();
    double loss = 0;
    size_t weight_change = 0;
    size_t weight_try = 0;
    size_t weight_try_side = 0;
};

CalcerParams MakeCalcerParams(int16_t real_feature_index, bool weights_reverted) {
    CalcerFeatureChangeType change_type =
        real_feature_index < q_core::NUMBER_OF_PIECES
            ? CalcerFeatureChangeType::PieceWeight
            : (real_feature_index < q_core::NUMBER_OF_PIECES + COMPRESSED_PSQ_SIZE
                   ? CalcerFeatureChangeType::PSQ
                   : CalcerFeatureChangeType::Model);
    q_core::Piece piece =
        change_type == CalcerFeatureChangeType::Model
            ? q_core::Piece::King
            : (change_type == CalcerFeatureChangeType::PSQ
                   ? static_cast<q_core::Piece>(
                         (real_feature_index - q_core::NUMBER_OF_PIECES) / q_core::BOARD_SIZE + 1)
                   : static_cast<q_core::Piece>(real_feature_index + 1));
    q_core::coord_t coord =
        change_type != CalcerFeatureChangeType::PSQ
            ? q_core::UNDEFINED_COORD
            : (real_feature_index - q_core::NUMBER_OF_PIECES) % q_core::BOARD_SIZE;
    return CalcerParams{.changed_piece = piece,
                        .changed_coord = coord,
                        .change_type = change_type,
                        .revert_last_change = weights_reverted};
}

void TuneWeights(const LearnerParams& params) {
    std::shared_ptr<Dataset> dataset = params.dataset;

    std::array<int16_t, PSQ_AND_FEATURE_COUNT> weights{};
    std::array<double, PSQ_AND_FEATURE_COUNT> grad_preference{};
    std::array<double, PSQ_AND_FEATURE_COUNT> improvements{};
    bool loaded_state = false;
    if (!params.state_filename.empty()) {
        loaded_state =
            ReadStateFromFile(weights, grad_preference, improvements, params.state_filename);
    }
    if (!loaded_state) {
        GetDefaultState(weights, grad_preference, improvements);
    }

    std::vector<std::shared_ptr<Element>> batch = dataset->GetAllElements();

    std::priority_queue<FeatureWithImprovement> priority{};
    for (size_t i = 0; i < PSQ_AND_FEATURE_COUNT; i++) {
        priority.push({.improvement = improvements[i], .num = i});
    }
    std::vector<FeatureWithImprovement> tried{};

    CalcerParams calcer_params{.changed_piece = q_core::Piece::Pawn,
                               .changed_coord = q_core::UNDEFINED_COORD,
                               .change_type = CalcerFeatureChangeType::Model,
                               .revert_last_change = false};
    bool weights_reverted = false;
    double best_loss = GetLossByWeights(params, batch, weights, calcer_params);

    const auto start_time = std::chrono::steady_clock::now();
    Stat stat;
    stat.loss = best_loss;
    while (!priority.empty()) {
        size_t index = priority.top().num;
        double improvement = priority.top().improvement;
        priority.pop();
        stat.weight_try++;

        bool improved = false;
        double old_loss = best_loss;
        const int grad_sign = grad_preference[index] >= 0 ? 1 : -1;
        grad_preference[index] *= 0.8;
        for (int delta = 1; delta >= -1; delta -= 2) {
            stat.weight_try_side++;
            int16_t weight_delta = delta * grad_sign;
            weights[index] += weight_delta;

            int16_t real_feature_index =
                index >= TAPERED_EVAL_BOUND ? index - TAPERED_EVAL_BOUND : index;
            calcer_params = MakeCalcerParams(real_feature_index, weights_reverted);

            double cur_loss = GetLossByWeights(params, batch, weights, calcer_params);
            if (cur_loss + 1e-12 < best_loss) {
                best_loss = cur_loss;
                improved = true;
                grad_preference[index] += weight_delta;
                stat.weight_change++;
                weights_reverted = false;
                break;
            }
            weights[index] -= weight_delta;
            weights_reverted = true;
        }
        if (improved) {
            double delta = old_loss - best_loss;
            improvements[index] = improvement * 0.9 + delta;
            priority.push({.improvement = improvement * 0.9 + delta, .num = index});
            for (const auto& i : tried) {
                priority.push({.improvement = i.improvement, .num = i.num});
            }
            tried.clear();
        } else {
            improvements[index] = improvement * 0.9;
            tried.push_back({.improvement = improvement * 0.9, .num = index});
        }
        double last_print_duration =
            static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::steady_clock::now() - stat.time)
                                    .count()) /
            1000;
        if (last_print_duration > 5) {
            double time_from_start =
                static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::steady_clock::now() - start_time)
                                        .count()) /
                1000 / 60;
            q_util::Print("Element count:", batch.size(), "loss change:", stat.loss - best_loss,
                          "loss:", best_loss, "changes:", stat.weight_change, "/", stat.weight_try,
                          "/", stat.weight_try_side, "time:", time_from_start, "min");
            WriteResultToFile(weights, params.output_filename);
            if (!params.state_filename.empty()) {
                WriteStateToFile(weights, grad_preference, improvements, params.state_filename);
            }
            stat = {};
            stat.loss = best_loss;
        }
    }
}
