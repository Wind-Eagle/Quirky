#ifndef QUIRKY_SRC_EVAL_LAYERS_H
#define QUIRKY_SRC_EVAL_LAYERS_H

// Code is inherited from:
// https://github.com/official-stockfish/nnue-pytorch/blob/master/docs/nnue.md

#include "util/bit.h"
#include "util/macro.h"
#ifndef NO_AVX2
#include <immintrin.h>
#endif

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "model_weights.h"

namespace q_eval {

static constexpr int WEIGHT_SCALE = 64;
static constexpr int ACTIVATION_SCALE = 127;
static constexpr int OUTPUT_SCALE = 64 * 64;

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

  private:
    alignas(64) std::array<std::array<int16_t, OUTPUT_SIZE>, INPUT_SIZE> weights_;
    alignas(64) std::array<int16_t, OUTPUT_SIZE> biases_;
};

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE>
struct LinearLayer {
  public:
    void Initialize(ModelReader& reader) {
        for (size_t i = 0; i < INPUT_SIZE; i++) {
            for (size_t j = 0; j < OUTPUT_SIZE; j++) {
                if (i >= 16 && INPUT_SIZE == 32) {
                    weights_[j * INPUT_SIZE + i] = 0;
                    continue;
                }
                weights_[j * INPUT_SIZE + i] = reader.ReadWeight<int8_t>(WEIGHT_SCALE);
            }
        }
        for (size_t i = 0; i < OUTPUT_SIZE; i++) {
            biases_[i] = reader.ReadWeight<int32_t>(ACTIVATION_SCALE * WEIGHT_SCALE);
        }
    }

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

                Multiply(sum0, in,
                         _mm256_load_si256((__m256i*)&weights_[offset0 + j * REGISTER_WIDTH]));
                Multiply(sum1, in,
                         _mm256_load_si256((__m256i*)&weights_[offset1 + j * REGISTER_WIDTH]));
                Multiply(sum2, in,
                         _mm256_load_si256((__m256i*)&weights_[offset2 + j * REGISTER_WIDTH]));
                Multiply(sum3, in,
                         _mm256_load_si256((__m256i*)&weights_[offset3 + j * REGISTER_WIDTH]));
            }
            const __m128i bias = _mm_load_si128((__m128i*)&biases_[i * 4]);
            __m128i outval = Add(sum0, sum1, sum2, sum3, bias);
            _mm_store_si128((__m128i*)&output[i * 4], outval);
        }
    }

  private:
    void Multiply(__m256i& acc, __m256i a, __m256i b) {
        /*__m256i product0 = _mm256_maddubs_epi16(a, b);
        __m256i one = _mm256_set1_epi16(1);
        product0 = _mm256_madd_epi16(product0, one);
        acc = _mm256_add_epi32(acc, product0);*/
        acc = _mm256_dpbusd_epi32(acc, a, b);
    }

    __m128i Add(__m256i sum0, __m256i sum1, __m256i sum2, __m256i sum3, __m128i bias) {
        sum0 = _mm256_hadd_epi32(sum0, sum1);
        sum2 = _mm256_hadd_epi32(sum2, sum3);

        sum0 = _mm256_hadd_epi32(sum0, sum2);

        __m128i sum128lo = _mm256_castsi256_si128(sum0);
        __m128i sum128hi = _mm256_extracti128_si256(sum0, 1);

        return _mm_add_epi32(_mm_add_epi32(sum128lo, sum128hi), bias);
    }

    alignas(32) std::array<int8_t, INPUT_SIZE * OUTPUT_SIZE> weights_;
    alignas(32) std::array<int32_t, OUTPUT_SIZE> biases_;
};

template <size_t INPUT_SIZE>
struct OutputLayer {
  public:
    void Initialize(ModelReader& reader) {
        for (size_t i = 0; i < INPUT_SIZE; i++) {
            weights_[i] =
                reader.ReadWeight<int16_t>(WEIGHT_SCALE * OUTPUT_SCALE / ACTIVATION_SCALE);
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
    alignas(64) std::array<int16_t, INPUT_SIZE> weights_;
    int32_t bias_;
};

inline void ClippedReLU16(int size, int8_t* output, const int16_t* input) {
    constexpr int IN_REGISTER_WIDTH = 256 / 16;
    constexpr int OUT_REGISTER_WIDTH = 256 / 8;
    const int num_out_chunks = size / OUT_REGISTER_WIDTH;

    const __m256i zero = _mm256_setzero_si256();
    const int control = 0b11011000;

    for (int i = 0; i < num_out_chunks; ++i) {
        const __m256i in0 =
            _mm256_load_si256((const __m256i*)&input[(i * 2 + 0) * IN_REGISTER_WIDTH]);
        const __m256i in1 =
            _mm256_load_si256((const __m256i*)&input[(i * 2 + 1) * IN_REGISTER_WIDTH]);

        const __m256i result =
            _mm256_permute4x64_epi64(_mm256_max_epi8(_mm256_packs_epi16(in0, in1), zero), control);

        _mm256_store_si256((__m256i*)&output[i * OUT_REGISTER_WIDTH], result);
    }
}

inline void ClippedReLU32(int size, int8_t* output, const int32_t* input) {
    constexpr int IN_REGISTER_WIDTH = 256 / 32;
    constexpr int OUT_REGISTER_WIDTH = 256 / 8;
    const int num_out_chunks = size / OUT_REGISTER_WIDTH;

    const __m256i zero = _mm256_setzero_si256();
    const __m256i control = _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0);

    for (int i = 0; i < num_out_chunks; ++i) {
        const __m256i in0 = _mm256_packs_epi32(
            _mm256_load_si256((const __m256i*)&input[(i * 4 + 0) * IN_REGISTER_WIDTH]),
            _mm256_load_si256((const __m256i*)&input[(i * 4 + 1) * IN_REGISTER_WIDTH]));
        const __m256i in1 = _mm256_packs_epi32(
            _mm256_load_si256((const __m256i*)&input[(i * 4 + 2) * IN_REGISTER_WIDTH]),
            _mm256_load_si256((const __m256i*)&input[(i * 4 + 3) * IN_REGISTER_WIDTH]));

        const __m256i result = _mm256_permutevar8x32_epi32(
            _mm256_max_epi8(_mm256_packs_epi16(in0, in1), zero), control);

        _mm256_store_si256((__m256i*)&output[i * OUT_REGISTER_WIDTH], result);
    }
}

template <uint32_t div>
void QuantDiv32(size_t size, int32_t* input) {
    Q_STATIC_ASSERT(div > 0 && q_util::GetBitCount(div) == 1);
    constexpr int DIV_VAL = q_util::GetLowestBit(div);
    for (size_t i = 0; i < size; i += 8) {
        __m256i data = _mm256_loadu_si256((__m256i const*)(input + i));
        __m256i result = _mm256_srai_epi32(data, DIV_VAL);
        _mm256_storeu_si256((__m256i*)(input + i), result);
    }
}

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
