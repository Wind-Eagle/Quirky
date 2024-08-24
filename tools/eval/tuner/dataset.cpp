#include "dataset.h"

#include "../../../src/util/processor.h"
#include "../../../src/util/random.h"

void Dataset::Load(Reader& reader) {
    while (reader.HasNext()) {
        Game game = reader.GetNextGame();
        if (elements_.size() < 25000) {
            std::vector<PositionState> states(game.moves.size(), PositionState{.old_score = q_eval::SCORE_UNKNOWN, .new_score = q_eval::SCORE_UNKNOWN, .score_type = NotReady, .force_update = true});
            Element element{.game = game, .states = states};
            elements_.push_back(std::make_shared<Element>(element));
        } else {
            break;
        }
    }
}

std::vector<std::shared_ptr<Element>> Dataset::GetAllElements() const {
    return elements_;
}

size_t Dataset::Size() const {
    return elements_.size();
}
 
