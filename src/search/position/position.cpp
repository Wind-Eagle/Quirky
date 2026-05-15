#include "position.h"

#include "core/moves/attack.h"
#include "core/board/types.h"
#include "core/util.h"
#include "eval/evaluator.h"

namespace q_search {

Position::Position(const q_core::Board& b) {
    board = b;
    ConstructPosition();
}

Position::Position(const std::string_view s) {
    board.MakeFromFEN(s);
    ConstructPosition();
}

Position::~Position() {
    for (size_t i = 0; i < MAX_BUFFER_SIZE; i++) {
        delete buffer_[i];
    }
}

void Position::UnmakeMove(const q_core::Move move, const q_core::MakeMoveInfo& make_move_info) {
    evaluator.SetState(buffer_[--buffer_head_]);
    q_core::UnmakeMove(board, move, make_move_info);
}

bool Position::MakeMove(const q_core::Move move, q_core::MakeMoveInfo& make_move_info) {
    q_core::MakeMove(board, move, make_move_info);
    if (!q_core::WasMoveLegal(board, move)) {
        q_core::UnmakeMove(board, move, make_move_info);
        return false;
    }
    evaluator.UpdateOnMove(board, move, make_move_info, buffer_[++buffer_head_]);
    return true;
}

bool Position::MakeMove(q_core::Move move, q_core::MakeMoveInfo& make_move_info,
                        const std::function<void()>& after_board_change) {
    q_core::MakeMove(board, move, make_move_info);
    if (!q_core::WasMoveLegal(board, move)) {
        q_core::UnmakeMove(board, move, make_move_info);
        return false;
    }
    after_board_change();
    evaluator.UpdateOnMove(board, move, make_move_info,buffer_[++buffer_head_]);
    return true;
}

void Position::MakeNullMove(q_core::coord_t& old_en_passant_coord) {
    q_core::MakeNullMove(board, old_en_passant_coord);
}

void Position::UnmakeNullMove(const q_core::coord_t& old_en_passant_coord) {
    q_core::UnmakeNullMove(board, old_en_passant_coord);
}

void Position::ConstructPosition() {
    for (size_t i = 0; i < MAX_BUFFER_SIZE; i++) {
        buffer_[i] = new q_eval::Evaluator::State();
    }
    evaluator.StartTrackingBoard(board, buffer_[0]);
}

bool Position::IsCheck() const { return q_core::IsKingInCheck(board); }

q_eval::score_t Position::GetEvaluatorScore() const { return evaluator.Evaluate(board); }

bool Position::HasNonPawns() const {
    return board.bb_pieces[q_core::MakeCell(q_core::Color::White, q_core::Piece::Knight)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::White, q_core::Piece::Bishop)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::White, q_core::Piece::Rook)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::White, q_core::Piece::Queen)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::Black, q_core::Piece::Knight)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::Black, q_core::Piece::Bishop)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::Black, q_core::Piece::Rook)] |
           board.bb_pieces[q_core::MakeCell(q_core::Color::Black, q_core::Piece::Queen)];
}

bool Position::HasNonPawns(q_core::Color c) const {
    return board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Knight)] |
           board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Bishop)] |
           board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Rook)] |
           board.bb_pieces[q_core::MakeCell(c, q_core::Piece::Queen)];
}

}  // namespace q_search
