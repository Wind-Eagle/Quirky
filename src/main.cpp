#include "api/api.h"
#include "core/board/board.h"

int main() {
    core::Board board;
    board.BuildFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    api::StartCommunication();
}
