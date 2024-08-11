#include "dataset.h"

#include "../../../src/util/processor.h"
#include "../../../src/util/random.h"

void Dataset::Load(Reader& reader) {
    while (reader.HasNext()) {
        elements_.push_back(reader.GetNextGame());
    }
}

std::vector<Game> Dataset::GetBatch(size_t batch_size) {
    std::vector<Game> games;
    for (size_t i = 0; i < batch_size; i++) {
        games.push_back(elements_[q_util::GetRandom64() % elements_.size()]);
    }
    return games;
}
 
