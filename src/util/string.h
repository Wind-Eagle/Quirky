#ifndef QUIRKY_SRC_UTIL_STRING_H
#define QUIRKY_SRC_UTIL_STRING_H

#include <algorithm>
#include <string>
#include <vector>

namespace q_util {

template <class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

template <StringLike T>
inline std::vector<std::string> SplitString(const T& arg, const char delim = ' ') {
    std::vector<std::string> res;
    std::string_view str = static_cast<std::string_view>(arg);
    size_t start = 0;
    size_t end;
    while ((end = str.find(delim, start)) != std::string::npos) {
        res.push_back(std::string(str.substr(start, end - start)));
        start = end + 1;
    }
    res.push_back(std::string(str.substr(start, end - start)));
    return res;
}

inline std::string ConcatenateStrings(const auto& begin_iterator, const auto& end_iterator,
                                      const char delim = ' ') {
    std::string res;
    for (auto i = begin_iterator; i != end_iterator; i++) {
        if (!res.empty()) {
            res += delim;
        }
        res += std::string(*i);
    }
    return res;
}

template <StringLike T>
inline bool IsStringNonNegativeNumber(const T& s) {
    return !s.empty() && std::find_if(s.begin(), s.end(),
                                      [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

template <StringLike T>
inline bool IsStringNumber(const T& s) {
    return !s.empty() && std::find_if(s[0] == '-' ? s.begin() + 1 : s.begin(), s.end(),
                                      [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_STRING_H
