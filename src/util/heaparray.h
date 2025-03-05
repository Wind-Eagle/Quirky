#ifndef QUIRKY_SRC_UTIL_HEAPARRAY_H
#define QUIRKY_SRC_UTIL_HEAPARRAY_H

#include <array>
#include <memory>

namespace q_util {

template <class T, size_t Size>
class HeapArray {
  public:
    HeapArray() { array_ = std::make_unique<std::array<T, Size>>(); }
    HeapArray(const HeapArray& rhs) {
        std::array<T, Size> copy = *rhs.array_.get();
        array_ = std::make_unique<std::array<T, Size>>(copy);
    }
    HeapArray(HeapArray&& rhs) noexcept { array_ = std::move(rhs.array_); }
    HeapArray& operator=(const HeapArray& rhs) {
        std::array<T, Size> copy = *rhs.array_.get();
        array_ = std::make_unique<std::array<T, Size>>(copy);
        return *this;
    }
    HeapArray& operator=(HeapArray&& rhs) noexcept {
        array_ = std::move(rhs.array_);
        return *this;
    }
    void Clear() { *array_ = std::array<T, Size>{}; }
    std::array<T, Size>& GetArrayRef() { return *array_; }
    const std::array<T, Size>& GetArrayRef() const { return *array_; }

  private:
    std::unique_ptr<std::array<T, Size>> array_;
};

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_IO_H
