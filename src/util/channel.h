#ifndef QUIRKY_SRC_UTIL_CHANNEL_H
#define QUIRKY_SRC_UTIL_CHANNEL_H

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace q_util {

template <class T>
class BufferedChannel {
  public:
    explicit BufferedChannel(uint32_t size) { max_size_ = size; }

    void Send(T value) {
        std::unique_lock<std::mutex> lock(sync_mutex_);
        cv_big_.wait(lock, [this] { return size_ < max_size_; });
        if (closed_) {
            throw std::runtime_error("Channel is closed");
        }
        queue_.push(std::forward<T>(value));
        size_++;
        cv_small_.notify_all();
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(sync_mutex_);
        cv_small_.wait(lock, [this] { return closed_ || size_ > 0; });
        if (closed_ && size_ == 0) {
            return std::nullopt;
        }
        auto element = std::forward<T>(queue_.front());
        queue_.pop();
        size_--;
        cv_big_.notify_all();
        return element;
    }

    void Close() {
        std::unique_lock<std::mutex> lock(sync_mutex_);
        closed_ = true;
        cv_small_.notify_all();
        cv_big_.notify_all();
    }

  private:
    uint32_t max_size_;
    uint32_t size_ = 0;
    bool closed_ = false;
    std::queue<T> queue_;
    std::mutex sync_mutex_;
    std::condition_variable cv_small_;
    std::condition_variable cv_big_;
};

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_CHANNEL_H
