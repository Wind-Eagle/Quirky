#include "api.h"
#include "uci/uci_protocol.h"
#include "xboard/xboard_protocol.h"
#include "../util/io.h"

#include <iostream>

namespace api {

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
        io::Print("Undefined protocol name: use either uci or xboard.");
    }
}

}  // namespace api