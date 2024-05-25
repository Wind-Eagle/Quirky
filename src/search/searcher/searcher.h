#ifndef QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
#define QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H

#include "../position/position.h"

namespace q_search {

class Searcher {
    public:
        static constexpr uint8_t MAX_DEPTH = 127;
    private:
        q_search::Position position_;
        
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_SEARCHER_SEARCHER_H
