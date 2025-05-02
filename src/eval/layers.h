#ifndef QUIRKY_SRC_EVAL_LAYERS_H
#define QUIRKY_SRC_EVAL_LAYERS_H

// Code is inherited from:
// https://github.com/official-stockfish/nnue-pytorch/blob/master/docs/nnue.md

#ifndef NO_AVX2
#include <immintrin.h>
#endif

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "../util/macro.h"
#include "model_weights.h"

namespace q_eval {

static constexpr float WEIGHT_SCALE = 64 * 64;
static constexpr float ACTIVATION_SCALE = 127;
static constexpr float OUTPUT_SCALE = 64;

struct ModelReader {
  public:
    template <std::integral T>
    T ReadWeight(int scale) {
        float weight = MODEL_WEIGHTS[index_++];
        int64_t final_weight = std::round(weight * scale);
        final_weight = std::min(final_weight, static_cast<int64_t>(std::numeric_limits<T>::max()));
        final_weight = std::max(final_weight, static_cast<int64_t>(std::numeric_limits<T>::min()));
        return final_weight;
    }

  private:
    size_t index_ = 0;
};

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE>
struct FeatureLayer {
  public:
    void Initialize(ModelReader& reader) {
        for (size_t i = 0; i < INPUT_SIZE; i++) {
            for (size_t j = 0; j < OUTPUT_SIZE; j++) {
                weights_[i][j] = reader.ReadWeight<int16_t>(ACTIVATION_SCALE);
            }
        }
        for (size_t i = 0; i < OUTPUT_SIZE; i++) {
            biases_[i] = reader.ReadWeight<int16_t>(ACTIVATION_SCALE);
        }
    }

    void GetResultOnEmptyBoard(int16_t* output) {
        std::copy(biases_.begin(), biases_.end(), output);
    }

    #ifndef NO_AVX2
    void Update(int16_t* input, size_t position, int8_t delta) {
        static constexpr int REGISTER_WIDTH = 256 / 16;
        constexpr int NUMBER_OF_CHUNKS = OUTPUT_SIZE / REGISTER_WIDTH;
        __m256i regs[NUMBER_OF_CHUNKS];
        for (uint16_t i = 0; i < NUMBER_OF_CHUNKS; i++) {
            regs[i] = _mm256_load_si256((__m256i*)(&input[i * REGISTER_WIDTH]));
        }
        if (delta == -1) {
            for (uint16_t i = 0; i < NUMBER_OF_CHUNKS; i++) {
                regs[i] = _mm256_sub_epi16(
                    regs[i],
                    _mm256_load_si256((__m256i*)(&weights_[position][i * REGISTER_WIDTH])));
            }
        } else {
            for (uint16_t i = 0; i < NUMBER_OF_CHUNKS; i++) {
                regs[i] = _mm256_add_epi16(
                    regs[i],
                    _mm256_load_si256((__m256i*)(&weights_[position][i * REGISTER_WIDTH])));
            }
        }
        for (uint16_t i = 0; i < NUMBER_OF_CHUNKS; i++) {
            _mm256_store_si256((__m256i*)(&input[i * REGISTER_WIDTH]), regs[i]);
        }
    }
    #else
    void Update(int16_t* __restrict input, size_t position, int8_t delta) {
        const int16_t* __restrict weights = weights_[position].data();
        if (delta == -1) {
            for (uint16_t i = 0; i < OUTPUT_SIZE; i++) {
                input[i] -= weights[i];
            }
        } else {
            for (uint16_t i = 0; i < OUTPUT_SIZE; i++) {
                input[i] += weights[i];
            }
        }
    }
    #endif

  private:
    alignas(32) std::array<std::array<int16_t, OUTPUT_SIZE>, INPUT_SIZE> weights_;
    alignas(32) std::array<int16_t, OUTPUT_SIZE> biases_;
};

template <size_t INPUT_SIZE>
struct OutputLayer {
  public:
    void Initialize(ModelReader& reader) {
        for (size_t i = 0; i < INPUT_SIZE; i++) {
            weights_[i] = reader.ReadWeight<int16_t>(WEIGHT_SCALE * OUTPUT_SCALE / ACTIVATION_SCALE);
        }
        bias_ = reader.ReadWeight<int32_t>(WEIGHT_SCALE * OUTPUT_SCALE);
    }

    int32_t Process(const int8_t* input) {
        int32_t ans = 0;
        for (uint16_t i = 0; i < INPUT_SIZE; i++) {
            ans += static_cast<int32_t>(input[i]) * static_cast<int32_t>(weights_[i]);
        }
        ans += bias_;
        return ans;
    }

  private:
    alignas(32) std::array<int16_t, INPUT_SIZE> weights_;
    int32_t bias_;
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
