#ifndef QUIRKY_SRC_UTIL_ERROR_H
#define QUIRKY_SRC_UTIL_ERROR_H

#include <cstdint>

enum class QuirkyError : int32_t { UnknownError = -1, UnexpectedValue = 1, UnexpectedArgument = 2, ParseError = 3 };

inline constexpr const char* GetErrorMessage(const QuirkyError error) {
    switch (error) {
        case QuirkyError::UnknownError:
            return "Unknown error";
        case QuirkyError::UnexpectedValue:
            return "Unexpected value";
        case QuirkyError::UnexpectedArgument:
            return "Unexpected argument";
        case QuirkyError::ParseError:
            return "Unexpected argument";
        default:
            return "Can't get error message";
    }
}

#endif  // QUIRKY_SRC_UTIL_ERROR_H
