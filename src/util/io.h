#ifndef QUIRKY_SRC_UTIL_IO_H
#define QUIRKY_SRC_UTIL_IO_H

#include <iostream>

namespace q_util {

inline void Print() { std::cout << std::endl; }

template <class First, class... Rest>
inline void Print(First first, Rest... rest) {
    std::cout << first;
    if constexpr (sizeof...(rest) > 0) {
        std::cout << " ";
    }
    Print(rest...);
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_IO_H
