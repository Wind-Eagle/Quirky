#ifndef QUIRKY_SRC_EVAL_LAYERS_H
#define QUIRKY_SRC_EVAL_LAYERS_H

// Code is inherited from:
// https://github.com/official-stockfish/nnue-pytorch/blob/master/docs/nnue.md
// https://github.com/jhonnold/berserk/blob/main/src/nn/evaluate.c

// Compilation without avx2 is currently not supported

#include <immintrin.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "core/board/types.h"
#include "core/util.h"
#include "model_weights.h"
#include "util/bit.h"

namespace q_eval {

static constexpr int WEIGHT_SCALE = 64;
static constexpr int ACTIVATION_SCALE = 127;
static constexpr int OUTPUT_SCALE = 64 * 64;
static constexpr int PRECISE_WEIGHT_SCALE = 64;

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
            for (size_t j = 0; j < OUTPUT_SIZE / 2; j++) {
                weights_[i][j] = reader.ReadWeight<int16_t>(ACTIVATION_SCALE);
                const q_core::cell_t cell = i / q_core::BOARD_SIZE + 1;
                const q_core::coord_t coord = i % q_core::BOARD_SIZE;
                size_t pos =
                    (static_cast<size_t>(q_core::FlipCellColor(cell)) - 1) * q_core::BOARD_SIZE +
                    q_core::FlipCoord(coord);
                weights_[pos][j + OUTPUT_SIZE / 2] = weights_[i][j];
            }
        }
        for (size_t i = 0; i < OUTPUT_SIZE / 2; i++) {
            biases_[i] = reader.ReadWeight<int16_t>(ACTIVATION_SCALE);
            biases_[i + OUTPUT_SIZE / 2] = biases_[i];
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

    void SubAdd(int16_t* __restrict input, size_t position_first, size_t position_second) {
        for (uint16_t i = 0; i < OUTPUT_SIZE; i++) {
            input[i] += weights_[position_second][i] - weights_[position_first][i];
        }
    }

    void SubSubAdd(int16_t* __restrict input, size_t position_first, size_t position_second, size_t position_third) {
        for (uint16_t i = 0; i < OUTPUT_SIZE; i++) {
            input[i] += weights_[position_third][i] - weights_[position_first][i] - weights_[position_second][i];
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
                weights_[GetWeightIndex(j * INPUT_SIZE + i)] =
                    reader.ReadWeight<int8_t>(WEIGHT_SCALE * 2);
            }
        }
        for (size_t i = 0; i < OUTPUT_SIZE; i++) {
            biases_[i] = reader.ReadWeight<int32_t>(ACTIVATION_SCALE * WEIGHT_SCALE * 2);
        }
        for (size_t i = 0; i < 256; i++) {
            for (size_t j = 0; j < 8; j++) {
                lookup_indices_[i][j] = 0;
            }
            uint64_t j = i;
            uint64_t k = 0;
            while (j > 0) {
                lookup_indices_[i][k++] = q_util::ExtractLowestBit(j);
            }
        }
    }

    void Process(const int8_t* src, int32_t* dest) {
        constexpr size_t SPARSE_CHUNK_SIZE = 4;
        constexpr size_t OUT_WIDTH = sizeof(__m256i) / sizeof(int32_t);
        constexpr size_t NUM_CHUNKS = INPUT_SIZE / SPARSE_CHUNK_SIZE;
        constexpr size_t OUT_CC = OUTPUT_SIZE / OUT_WIDTH;

        const int32_t* in32 = (const int32_t*)src;
        const __m256i* biases = (__m256i*)biases_.data();
        __m256i* out = (__m256i*)dest;

        uint16_t nnz[NUM_CHUNKS];
        size_t count = FindNNZ(nnz, in32, NUM_CHUNKS);

        __m256i regs[OUT_CC];
        for (size_t i = 0; i < OUT_CC; i++) regs[i] = biases[i];

        size_t i = 0;
        for (; i + 1 < count; i += 2) {
            const uint16_t i0 = nnz[i + 0];
            const uint16_t i1 = nnz[i + 1];

            const __m256i f0 = _mm256_set1_epi32(in32[i0]);
            const __m256i f1 = _mm256_set1_epi32(in32[i1]);

            const __m256i* c0 = (__m256i*)&weights_[i0 * OUTPUT_SIZE * SPARSE_CHUNK_SIZE];
            const __m256i* c1 = (__m256i*)&weights_[i1 * OUTPUT_SIZE * SPARSE_CHUNK_SIZE];

            for (size_t j = 0; j < OUT_CC; j++) Addx2(regs + j, f0, c0[j], f1, c1[j]);
        }

        if (i < count) {
            const uint16_t i0 = nnz[i];
            const __m256i f0 = _mm256_set1_epi32(in32[i0]);
            const __m256i* c0 = (__m256i*)&weights_[i0 * OUTPUT_SIZE * SPARSE_CHUNK_SIZE];

            for (size_t j = 0; j < OUT_CC; j++) {
                Add(regs + j, f0, c0[j]);
            }
        }

        for (i = 0; i < OUT_CC; i++) {
            out[i] = _mm256_srai_epi32(regs[i], 1);
        }
    }

  private:
    int GetWeightIndex(int idx) {
        return ((idx / 4) % (INPUT_SIZE / 4) * OUTPUT_SIZE * 4) + (idx / INPUT_SIZE * 4) +
               (idx % 4);
    }
    void Add(__m256i* acc, __m256i a, __m256i b) {
        __m256i p0 = _mm256_maddubs_epi16(a, b);
        p0 = _mm256_madd_epi16(p0, _mm256_set1_epi16(1));
        *acc = _mm256_add_epi32(*acc, p0);
    }

    void Addx2(__m256i* acc, __m256i a0, __m256i b0, __m256i a1, __m256i b1) {
        __m256i p0 = _mm256_maddubs_epi16(a0, b0);
        __m256i p1 = _mm256_maddubs_epi16(a1, b1);

        p0 = _mm256_madd_epi16(_mm256_add_epi16(p0, p1), _mm256_set1_epi16(1));
        *acc = _mm256_add_epi32(*acc, p0);
    }

    uint32_t NNZ(__m256i chunk) {
        return _mm256_movemask_ps(
            _mm256_castsi256_ps(_mm256_cmpgt_epi32(chunk, _mm256_setzero_si256())));
    }

    size_t FindNNZ(uint16_t* dest, const int32_t* inputs, const size_t chunks) {
        constexpr size_t IN_WIDTH = sizeof(__m256i) / sizeof(int32_t);
        constexpr size_t CHUNK_SIZE = IN_WIDTH;
        const size_t num_chunks = chunks / CHUNK_SIZE;
        constexpr size_t IN_PER_CHUNK = CHUNK_SIZE / IN_WIDTH;
        constexpr size_t OUT_PER_CHUNK = CHUNK_SIZE / 8;

        const __m256i* in = (const __m256i*)inputs;

        size_t count = 0;

        const __m128i increment = _mm_set1_epi16(8);
        __m128i base = _mm_setzero_si128();

        for (size_t i = 0; i < num_chunks; i++) {
            uint32_t nnz = 0;

            for (size_t j = 0; j < IN_PER_CHUNK; j++) {
                const __m256i input_chunk = in[i * IN_PER_CHUNK + j];
                nnz |= NNZ(input_chunk) << (j * IN_WIDTH);
            }

            for (size_t j = 0; j < OUT_PER_CHUNK; j++) {
                const uint16_t lookup = (nnz >> (j * 8)) & 0xFF;
                const __m128i offsets = _mm_loadu_si128((__m128i*)(lookup_indices_[lookup].data()));
                _mm_storeu_si128((__m128i*)(dest + count), _mm_add_epi16(base, offsets));
                count += q_util::GetBitCount(lookup);
                base = _mm_add_epi16(base, increment);
            }
        }

        return count;
    }

    alignas(64) std::array<int8_t, INPUT_SIZE * OUTPUT_SIZE> weights_;
    alignas(64) std::array<int32_t, OUTPUT_SIZE> biases_;
    alignas(64) std::array<std::array<uint16_t, 8>, 256> lookup_indices_;
};

template <size_t INPUT_SIZE, size_t OUTPUT_SIZE>
struct PreciseLinearLayer {
  public:
    void Initialize(ModelReader& reader) {
        for (size_t i = 0; i < INPUT_SIZE; i++) {
            for (size_t j = 0; j < OUTPUT_SIZE; j++) {
                weights_[j * INPUT_SIZE + i] =
                    reader.ReadWeight<int16_t>(WEIGHT_SCALE * PRECISE_WEIGHT_SCALE);
            }
        }
        for (size_t i = 0; i < OUTPUT_SIZE; i++) {
            biases_[i] = reader.ReadWeight<int32_t>(ACTIVATION_SCALE * WEIGHT_SCALE * WEIGHT_SCALE *
                                                    PRECISE_WEIGHT_SCALE);
        }
    }

    void Process(const int16_t* input, int32_t* output) {
        static constexpr int REGISTER_WIDTH = 256 / 16;
        constexpr int NUMBER_OF_INPUT_CHUNKS = INPUT_SIZE / REGISTER_WIDTH;
        constexpr int NUMBER_OF_OUTPUT_CHUNKS = OUTPUT_SIZE / 4;

        for (int i = 0; i < NUMBER_OF_OUTPUT_CHUNKS; i++) {
            const size_t offset0 = (i * 4 + 0) * INPUT_SIZE;
            const size_t offset1 = (i * 4 + 1) * INPUT_SIZE;
            const size_t offset2 = (i * 4 + 2) * INPUT_SIZE;
            const size_t offset3 = (i * 4 + 3) * INPUT_SIZE;

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
            outval = _mm_srai_epi32(
                outval,
                q_util::GetHighestBit(static_cast<uint32_t>(WEIGHT_SCALE * PRECISE_WEIGHT_SCALE)));
            _mm_store_si128((__m128i*)&output[i * 4], outval);
        }
    }

  private:
    void Multiply(__m256i& acc, __m256i a, __m256i b) {
        __m256i product = _mm256_madd_epi16(a, b);
        acc = _mm256_add_epi32(acc, product);
    }

    __m128i Add(__m256i sum0, __m256i sum1, __m256i sum2, __m256i sum3, __m128i bias) {
        sum0 = _mm256_hadd_epi32(sum0, sum1);
        sum2 = _mm256_hadd_epi32(sum2, sum3);
        sum0 = _mm256_hadd_epi32(sum0, sum2);

        __m128i sum128lo = _mm256_castsi256_si128(sum0);
        __m128i sum128hi = _mm256_extracti128_si256(sum0, 1);
        __m128i total = _mm_add_epi32(sum128lo, sum128hi);

        return _mm_add_epi32(total, bias);
    }
    alignas(64) std::array<int16_t, INPUT_SIZE * OUTPUT_SIZE> weights_;
    alignas(64) std::array<int32_t, OUTPUT_SIZE> biases_;
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

    int32_t Process(const int16_t* input) {
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

inline void ClippedReLU32(int size, int16_t* output, const int32_t* input) {
    const __m256i zero = _mm256_setzero_si256();
    const __m256i upper = _mm256_set1_epi32(32768 * WEIGHT_SCALE / 256 - 1);

    for (int i = 0; i < size; i += 16) {
        __m256i in0 = _mm256_loadu_si256((const __m256i*)(input + i));
        __m256i in1 = _mm256_loadu_si256((const __m256i*)(input + i + 8));

        __m256i clamped0 = _mm256_min_epi32(_mm256_max_epi32(in0, zero), upper);
        __m256i clamped1 = _mm256_min_epi32(_mm256_max_epi32(in1, zero), upper);

        __m256i packed = _mm256_packs_epi32(clamped0, clamped1);
        packed = _mm256_permute4x64_epi64(packed, 0b11011000);

        _mm256_storeu_si256((__m256i*)(output + i), packed);
    }
}

}  // namespace q_eval

#endif  // QUIRKY_SRC_EVAL_MODEL_H
