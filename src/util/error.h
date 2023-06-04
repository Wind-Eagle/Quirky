#ifndef QUIRKY_SRC_UTIL_ERROR_H
#define QUIRKY_SRC_UTIL_ERROR_H

enum class QuirkyError : int32_t { UnknownError = -1, UnexpectedValue = 1 };

inline constexpr const char* GetErrorMessage(QuirkyError error) {
    switch (error) {
        case QuirkyError::UnknownError:
            return "Unknown error";
        case QuirkyError::UnexpectedValue:
            return "Unexpected value";
        default:
            return "Can't get error message";
    }
}

#endif  // QUIRKY_SRC_UTIL_ERROR_H
