#ifndef QUIRKY_SRC_UTIL_IO_H
#define QUIRKY_SRC_UTIL_IO_H

#include <iostream>

#include "error.h"

namespace {

inline void PrintToStream(std::ostream& stream) { stream << std::endl; }

template <class First, class... Rest>
inline void PrintToStream(std::ostream& stream, First first, Rest... rest) {
    stream << first;
    if constexpr (sizeof...(rest) > 0) {
        stream << " ";
    }
    PrintToStream(stream, rest...);
}

}  // namespace

namespace q_util {

inline std::string ReadLine(std::istream& stream = std::cin) {
    std::string line;
    std::getline(stream, line);
    return line;
}

template <class First, class... Rest>
inline void Print(First first, Rest... rest) {
    PrintToStream(std::cout, first, rest...);
}

template <class First, class... Rest>
inline void LogText(std::ostream& stream, First first, Rest... rest) {
    PrintToStream(stream, "[", first, "]    ", rest...);
}

template <class First, class... Rest>
inline void PrintError(First first, Rest... rest) {
    PrintToStream(std::cerr, "[ERROR]    ", first, rest...);
}

inline void ExitWithError(QuirkyError error) {
    PrintToStream(std::cerr, "[FATAL ERROR]    ", GetErrorMessage(error));
    exit(static_cast<uint8_t>(error));
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_IO_H
