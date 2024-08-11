#ifndef QUIRKY_SRC_UTIL_PROCESSOR_H
#define QUIRKY_SRC_UTIL_PROCESSOR_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>

#include "channel.h"

namespace q_util {

template <class In, class Out>
class Processor {
  public:
    Processor(size_t number_of_threads, size_t receive_channel_size, size_t send_channel_size)
        : receive_channel_(receive_channel_size),
          send_channel_(send_channel_size),
          number_of_threads_(number_of_threads) {}
    void Start(std::function<Out(In)> func) {
        for (size_t i = 0; i < number_of_threads_; i++) {
            for (size_t i = 0; i < number_of_threads_; i++) {
                threads_.emplace_back([&]() {
                    for (;;) {
                        auto input = send_channel_.Recv();
                        if (input == std::nullopt) {
                            break;
                        }
                        receive_channel_.Send(func(input));
                    }
                });
            }
        }
    }
    void Stop() {
        receive_channel_.Close();
        send_channel_.Close();
    }
    void Send(In in) { send_channel_.Send(in); }
    std::optional<Out> Receive() { return receive_channel_.Recv(); }
    ~Processor() {
        for (size_t i = 0; i < number_of_threads_; i++) {
            threads_[i].join();
        }
    }

  private:
    std::vector<std::thread> threads_;
    q_util::BufferedChannel<Out> receive_channel_;
    q_util::BufferedChannel<In> send_channel_;
    size_t number_of_threads_;
};

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_PROCESSOR_H
