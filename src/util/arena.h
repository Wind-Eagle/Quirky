#ifndef QUIRKY_UTIL_ARENA_H
#define QUIRKY_UTIL_ARENA_H

#include <cstddef>
#include <memory>
#include <stack>
#include <vector>

#include "util/macro.h"

namespace q_util {

template <typename T, size_t BlockSize, size_t Alignment = std::alignment_of<T>::value>
class Arena {
  public:
    T* Allocate() {
        for (auto& block : blocks_) {
            if (block->HasSpace()) {
                return block->Allocate();
            }
        }
        blocks_.emplace_back(std::make_unique<Block>());
        return blocks_.back()->Allocate();
    }

    void Deallocate(T* ptr) {
        for (auto& block : blocks_) {
            if (block->IsOwnerOf(ptr)) {
                block->Free(ptr);
                return;
            }
        }
        Q_ASSERT(false);
    }

  private:
    class Block {
      public:
        Block() {
            for (size_t i = 0; i < BlockSize; ++i) {
                free_indices_.push(i);
            }
        }

        Block(const Block&) = delete;
        Block& operator=(const Block&) = delete;

        bool HasSpace() const { return !free_indices_.empty(); }

        T* Allocate() {
            T* result = items_.data() + free_indices_.top();
            free_indices_.pop();
            return result;
        }

        bool IsOwnerOf(T* ptr) const { return (size_t)(ptr - items_.data()) < items_.size(); }

        void Free(T* ptr) { free_indices_.push(ptr - items_.data()); }

      private:
        alignas(Alignment) std::array<T, BlockSize> items_;
        std::stack<size_t, std::vector<size_t>> free_indices_;
    };

    std::vector<std::unique_ptr<Block>> blocks_;
};

}  // namespace q_util

#endif
