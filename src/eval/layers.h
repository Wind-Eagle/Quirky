#ifndef QUIRKY_SRC_EVAL_LAYERS_H
#define QUIRKY_SRC_EVAL_LAYERS_H

// Code is inherited from: https://github.com/official-stockfish/nnue-pytorch/blob/master/docs/nnue.md

#include <immintrin.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include "../util/macro.h"

namespace q_eval {

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE>
struct FeatureLayer {
    alignas(32) std::array<std::array<int16_t, OUTPUT_SIZE>, INPUT_SIZE> weights;
    alignas(32) std::array<int16_t, OUTPUT_SIZE> biases;

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
                    regs[i], _mm256_load_si256((__m256i*)(&weights[position][i * REGISTER_WIDTH])));
            }
        } else {
            for (uint16_t i = 0; i < NUMBER_OF_CHUNKS; i++) {
                regs[i] = _mm256_add_epi16(
                    regs[i], _mm256_load_si256((__m256i*)(&weights[position][i * REGISTER_WIDTH])));
            }
        }
        for (uint16_t i = 0; i < NUMBER_OF_CHUNKS; i++) {
            _mm256_store_si256((__m256i*)(&input[i * REGISTER_WIDTH]), regs[i]);
        }
    }
};

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE>
struct LinearLayer {
  public:
    alignas(32) std::array<int8_t, INPUT_SIZE * OUTPUT_SIZE> weights;
    alignas(32) std::array<int32_t, OUTPUT_SIZE> biases;

  private:
    void Multiply(__m256i& acc, __m256i a, __m256i b) {
        __m256i product0 = _mm256_maddubs_epi16(a, b);
        __m256i one = _mm256_set1_epi16(1);
        product0 = _mm256_madd_epi16(product0, one);
        acc = _mm256_add_epi32(acc, product0);
    }

    __m128i Add(__m256i sum0, __m256i sum1, __m256i sum2, __m256i sum3, __m128i bias) {
        sum0 = _mm256_hadd_epi32(sum0, sum1);
        sum2 = _mm256_hadd_epi32(sum2, sum3);

        sum0 = _mm256_hadd_epi32(sum0, sum2);

        __m128i sum128lo = _mm256_castsi256_si128(sum0);
        __m128i sum128hi = _mm256_extracti128_si256(sum0, 1);

        return _mm_add_epi32(_mm_add_epi32(sum128lo, sum128hi), bias);
    }

  public:
    void Process(const int8_t* input, int32_t* output) {
        static constexpr int REGISTER_WIDTH = 256 / 8;
        constexpr int NUMBER_OF_INPUT_CHUNKS = INPUT_SIZE / REGISTER_WIDTH;
        constexpr int NUMBER_OF_OUTPUT_CHUNKS = OUTPUT_SIZE / 4;

        for (int i = 0; i < NUMBER_OF_OUTPUT_CHUNKS; i++) {
            const int offset0 = (i * 4 + 0) * INPUT_SIZE;
            const int offset1 = (i * 4 + 1) * INPUT_SIZE;
            const int offset2 = (i * 4 + 2) * INPUT_SIZE;
            const int offset3 = (i * 4 + 3) * INPUT_SIZE;

            __m256i sum0 = _mm256_setzero_si256();
            __m256i sum1 = _mm256_setzero_si256();
            __m256i sum2 = _mm256_setzero_si256();
            __m256i sum3 = _mm256_setzero_si256();
            for (int j = 0; j < NUMBER_OF_INPUT_CHUNKS; j++) {
                const __m256i in = _mm256_load_si256((const __m256i*)&input[j * REGISTER_WIDTH]);

                Multiply(
                    sum0, in, _mm256_load_si256((__m256i*)&weights[offset0 + j * REGISTER_WIDTH]));
                Multiply(
                    sum1, in, _mm256_load_si256((__m256i*)&weights[offset1 + j * REGISTER_WIDTH]));
                Multiply(
                    sum2, in, _mm256_load_si256((__m256i*)&weights[offset2 + j * REGISTER_WIDTH]));
                Multiply(
                    sum3, in, _mm256_load_si256((__m256i*)&weights[offset3 + j * REGISTER_WIDTH]));
            }
            const __m128i bias = _mm_load_si128((__m128i*)&biases[i * 4]);
            __m128i outval = Add(sum0, sum1, sum2, sum3, bias);
            _mm_store_si128((__m128i*)&output[i * 4], outval);
        }
    }
};


template <size_t INPUT_SIZE, size_t OUTPUT_SIZE>
struct SmallLinearLayer {
    alignas(32) std::array<std::array<int8_t, OUTPUT_SIZE>, INPUT_SIZE> weights;
    alignas(32) std::array<int32_t, OUTPUT_SIZE> biases;

    void Process(const int8_t* input, int32_t* buffer) {
        std::copy(biases.begin(), biases.end(), buffer);
        for (uint16_t i = 0; i < INPUT_SIZE; i++) {
            for (uint16_t j = 0; j < OUTPUT_SIZE; j++) {
                buffer[j] += static_cast<int16_t>(weights[i][j]) * input[i];
            }
        }
    }
};

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
