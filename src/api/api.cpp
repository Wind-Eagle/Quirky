#include "api.h"
#include "uci/uci_protocol.h"
#include "xboard/xboard_protocol.h"
#include "../util/io.h"

#include <iostream>

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
        q_util::Print("Unidentified protocol name: use either uci or xboard.");
    }
}

}  // namespace q_api