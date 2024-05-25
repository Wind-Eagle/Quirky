#ifndef QUIRKY_SRC_SEARCH_SEARCHER_LAUNCHER_H
#define QUIRKY_SRC_SEARCH_SEARCHER_LAUNCHER_H

#include "logger.h"
#include "searcher.h"
#include "../control/control.h"
#include "../position/position.h"
#include "../position/transposition_table.h"
#include "../position/repetition_table.h"

namespace q_search {

class SearchLauncher {
    public:
        
    private:
        static constexpr uint8_t TT_DEFAULT_BYTE_SIZE_LOG = 23;
        q_search::Searcher searcher_;
        q_search::SearchControl search_control_;
        q_search::SearchLogger search_logger_;
        q_search::TranspositionTable transposition_table_{TT_DEFAULT_BYTE_SIZE_LOG};
};

}  // namespace q_search

#endif  // QUIRKY_SRC_SEARCH_SEARCHER_LAUNCHER_H
