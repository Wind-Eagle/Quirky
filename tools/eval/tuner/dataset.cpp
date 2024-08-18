#include "dataset.h"

#include "../../../src/util/processor.h"
#include "../../../src/util/random.h"

void Dataset::Load(Reader& reader) {
    while (reader.HasNext()) {
        elements_.push_back(reader.GetNextGame());
    }
}

Game Dataset::GetElement(){
    return elements_[q_util::GetRandom64() % elements_.size()];
}

std::vector<Game> Dataset::GetAllElements() {
    return elements_;
}

size_t Dataset::Size() const {
    return elements_.size();
}
 
