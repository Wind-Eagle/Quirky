#ifndef QUIRKY_SRC_UTIL_MACRO_H
#define QUIRKY_SRC_UTIL_MACRO_H

#include <cassert>

#include "io.h"

#define ENUM_TO_INT_OP(type, base, op)                                          \
    inline constexpr type operator op(const type a, const type b) {             \
        return static_cast<type>(static_cast<base>(a) op static_cast<base>(b)); \
    }

#define ENUM_ASSIGNMENT_OP(type, op)                               \
    inline constexpr type &operator op##=(type &a, const type b) { \
        a = a op b;                                                \
        return a;                                                  \
    }

#define ENUM_TO_INT(type, base)   \
    ENUM_TO_INT_OP(type, base, &) \
    ENUM_TO_INT_OP(type, base, |) \
    ENUM_TO_INT_OP(type, base, ^) \
    ENUM_ASSIGNMENT_OP(type, &)   \
    ENUM_ASSIGNMENT_OP(type, |)   \
    ENUM_ASSIGNMENT_OP(type, ^)   \
    inline constexpr type operator~(const type a) { return type::All ^ a; }

#define Q_CONCAT_STRING_WITH_INT_HELPER(prefix, x) prefix #x
#define Q_CONCAT_STRING_WITH_INT(prefix, a) Q_CONCAT_STRING_WITH_INT_HELPER(prefix, a)

#define Q_UNIQUE_STRING(prefix) Q_CONCAT_STRING_WITH_INT(prefix, __COUNTER__)

#define Q_LIKELY(x) __builtin_expect(!!(x), 1)

#define Q_UNLIKELY(x) __builtin_expect(!!(x), 0)

#define Q_UNREACHABLE() __builtin_unreachable()

#define Q_ASSUME(x)          \
    do {                     \
        if (!(x)) {          \
            Q_UNREACHABLE(); \
        }                    \
    } while (false)

#define Q_PREFETCH(addr, ...)          \
    __builtin_prefetch(addr __VA_OPT__(,) __VA_ARGS__)

#define Q_ASSERT(condition) assert(Q_UNLIKELY(condition))

#define Q_STATIC_ASSERT(condition) static_assert(condition)

#define Q_EXPECT(condition, message)     \
    do {                                 \
        if (Q_UNLIKELY(!(condition))) {  \
            q_util::PrintError(message); \
        }                                \
    } while (false)

#define Q_FATAL_EXPECT(condition, error, message) \
    do {                                          \
        if (Q_UNLIKELY(!(condition))) {           \
            q_util::PrintError(message);          \
            q_util::ExitWithError(error);         \
        }                                         \
    } while (false)

#endif  // QUIRKY_SRC_UTIL_MACRO_H