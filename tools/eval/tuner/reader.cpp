#include "reader.h"

#include "../../../src/core/moves/board_manipulation.h"
#include "../../../src/core/moves/move.h"
#include "../../../src/util/string.h"

void SkipWhitespaces(std::ifstream& in) {
    std::string tmp;
    while (in.peek() == '\n') {
        std::getline(in, tmp);
    }
}

Reader::Reader(const std::string& filename) : in_(filename.data()) { SkipWhitespaces(in_); }

std::string ReadLine(std::ifstream& in) {
    SkipWhitespaces(in);
    std::string res;
    std::getline(in, res);
    return res;
}

Result CastStringToResult(const std::string_view str) {
    if (str == "W") {
        return Result::WhiteWins;
    }
    if (str == "D") {
        return Result::Draw;
    }
    if (str == "B") {
        return Result::BlackWins;
    }
    q_util::PrintError("Unexpected value as a result of the game");
    q_util::ExitWithError(QuirkyError::UnexpectedValue);
    return Result::Draw;
}

Result ReadGameResult(std::ifstream& in) {
    const std::string res = ReadLine(in);
    const auto words = q_util::SplitString(res, ' ');
    if (words.size() != 3) {
        q_util::PrintError("Expected three words in game header");
        q_util::ExitWithError(QuirkyError::ParseError);
    }
    if (words[0] != "game") {
        q_util::PrintError("Expected \"game\" word");
        q_util::ExitWithError(QuirkyError::ParseError);
    }
    // Title is currently unused
    const std::string title = ReadLine(in);
    return CastStringToResult(words[1]);
}

std::string ReadBoard(std::ifstream& in) {
    std::string fen = q_util::ReadLine(in);
    if (fen == "start") {
        fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }
    return fen.substr(6);
}

Game ReadGameWithResult(std::ifstream& in, size_t game_index) {
    const Result result = ReadGameResult(in);
    std::string start_board = ReadBoard(in);
    const std::string moves_str = ReadLine(in);
    const auto moves = q_util::SplitString(moves_str, ' ');
    if (moves[0] != "moves") {
        q_util::PrintError("Expected \"moves\" word");
        q_util::ExitWithError(QuirkyError::ParseError);
    }
    GameHeader header{.last_move_number = static_cast<uint16_t>(moves.size()),
                      .game_index = game_index,
                      .result = result};
    return Game{.start_board_fen = start_board, .moves = moves, .header = header};
}

Game Reader::ReadGame() { return ReadGameWithResult(in_, index_++); }

Game Reader::GetNextGame() {
    auto game = ReadGame();
    SkipWhitespaces(in_);
    return game;
}

bool Reader::HasNext() const { return !in_.eof(); }
