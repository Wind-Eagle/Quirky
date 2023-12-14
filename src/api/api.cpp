#include "api.h"

#include <iostream>

#include "../util/io.h"
#include "uci/protocol.h"
#include "xboard/protocol.h"

namespace q_api {

void StartCommunication() {
    for (;;) {
        std::string line;
        getline(std::cin, line);
        if (line == "uci") {
            StartUciProtocol();
            break;
        }
        if (line == "xboard") {
            StartXBoardProtocol();
            break;
        }
        q_util::Print("Unidentified protocol name: use uci, xboard or internal.");
    }
}

}  // namespace q_api