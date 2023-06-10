#ifndef QUIRKY_SRC_UTIL_STRING_H
#define QUIRKY_SRC_UTIL_STRING_H

#include <vector>

namespace q_util {

inline std::vector<std::string> SplitString(const std::string_view& str, const char delim = ' ') {
    std::vector<std::string> res;
    size_t start = 0;
    size_t end;
    while ((end = str.find(delim, start)) != std::string::npos) {
        res.push_back(std::string(str.substr(start, end - start)));
        start = end + 1;
    }
    res.push_back(std::string(str.substr(start, end - start)));
    return res;
}

}  // namespace q_util

#endif  // QUIRKY_SRC_UTIL_STRING_H
