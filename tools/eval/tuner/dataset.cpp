#include "dataset.h"

#include "../../../src/util/processor.h"
#include "../../../src/util/random.h"

std::vector<Element> MakeElements(const Game& game) {
    std::vector<Element> res;
    q_search::Position position;
    position.MakeFromFEN(game.start_board_fen);
    q_core::MakeMoveInfo make_move_info;
    q_eval::Evaluator<q_eval::EvaluationType::Value>::Tag evaluator_tag;
    for (size_t i = 0; i < game.moves.size(); i++) {
        if (i > 0) {
            position.MakeMove(q_core::TranslateStringToMove(position.board, game.moves[i]),
                              make_move_info, evaluator_tag);
        }
        res.push_back(Element{.position = position, .result = game.header.result});
    }
    return res;
}

void Dataset::Load(Reader& reader) {
    while (reader.HasNext()) {
        std::vector<Element> elements = MakeElements(reader.GetNextGame());
        for (const auto& element : elements) {
            elements_.push_back(std::make_shared<Element>(element));
        }
    }
}

std::vector<std::shared_ptr<Element>> Dataset::GetAllElements() const {
    return elements_;
}

size_t Dataset::Size() const {
    return elements_.size();
}
 
