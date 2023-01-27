#ifndef QUIRKY_SRC_UTIL_IO_H
#define QUIRKY_SRC_UTIL_IO_H

#include <iostream>

namespace util {

inline void Print() { std::cout << std::endl; }

template <class First, class... Rest>
inline void Print(First first, Rest... rest) {
    std::cout << first;
    if (sizeof...(rest) > 0) {
        std::cout << " ";
    }
    Print(rest...);
}

}  // namespace util

#endif  // QUIRKY_SRC_UTIL_IO_H
