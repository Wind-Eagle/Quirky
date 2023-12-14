#include "reader.h"
#include "../../../../src/util/string.h"
#include "../../../../src/core/moves/move.h"
#include "../../../../src/core/moves/board_manipulation.h"

#include <fstream>

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

std::string ReadLine(std::ifstream& in) {
    while (!in.eof()) {
        std::string res;
        std::getline(in, res);
        if (!res.empty()) {
            return res;
        }
    }
    q_util::PrintError("Unexpected end of file");
    q_util::ExitWithError(QuirkyError::ParseError);
    return "";
}

GameHeader ReadGameHeader(std::ifstream& in) {
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
    return GameHeader{.result = CastStringToResult(words[1]), .name = words[2]};
}

q_core::Board ReadBoard(std::ifstream& in) {
    std::string fen = ReadLine(in);
    if (fen == "start") {
        fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }
    q_core::Board res;
    res.MakeFromFEN(fen);
    return res;
}

GameStartposWithResult ReadGameStartposWithResult(std::ifstream& in) {
    return GameStartposWithResult{.header = ReadGameHeader(in), .board = ReadBoard(in)};
}

GameWithResult ReadGameWithResult(std::ifstream& in) {
    const GameStartposWithResult game_startpos = ReadGameStartposWithResult(in);
    const std::string res = ReadLine(in);
    const auto moves = q_util::SplitString(res, ' ');
    if (moves[0] != "moves") {
        q_util::PrintError("Expected \"moves\" word");
        q_util::ExitWithError(QuirkyError::ParseError);
    }
    std::vector<q_core::Board> boards = {game_startpos.board};
    q_core::Board cur_board = game_startpos.board;
    q_core::MakeMoveInfo info;
    for (size_t i = 1; i < moves.size(); i++) {
        q_core::Move move = q_core::TranslateStringToMove(cur_board, moves[i]);
        q_core::MakeMove(cur_board, move, info);
        boards.push_back(cur_board);
    }
    return GameWithResult{.header = game_startpos.header, .boards = boards};
}

GameSet ReadGames(const std::string_view path) {
    std::ifstream in(path.data());
    GameSet game_set;
    std::string tmp;
    while (true) {
        if (in.peek() == '\n') {
            std::getline(in, tmp);
            continue;
        }
        if (in.eof()) {
            break;
        }
        game_set.games.push_back(ReadGameWithResult(in));
    }
    game_set.file_path = path;
    return game_set;
}
