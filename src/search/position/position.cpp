#include "position.h"

#include "../../core/moves/attack.h"
#include "core/board/types.h"
#include "core/util.h"

namespace q_search {

void Position::UnmakeMove(const q_core::Move move, const q_core::MakeMoveInfo& make_move_info,
                          q_eval::Evaluator::EvaluatorUpdateInfo& evaluator_update_info) {
    evaluator.RevertOnMove(board, move, evaluator_update_info);
    q_core::UnmakeMove(board, move, make_move_info);
}

bool Position::MakeMove(const q_core::Move move, q_core::MakeMoveInfo& make_move_info,
                        q_eval::Evaluator::EvaluatorUpdateInfo& evaluator_update_info) {
    q_core::MakeMove(board, move, make_move_info);
    if (!q_core::WasMoveLegal(board, move)) {
        q_core::UnmakeMove(board, move, make_move_info);
        return false;
    }
    evaluator.UpdateOnMove(board, move, make_move_info, evaluator_update_info);
    return true;
}

bool Position::MakeMove(q_core::Move move, q_core::MakeMoveInfo& make_move_info,
                        q_eval::Evaluator::EvaluatorUpdateInfo& evaluator_update_info,
                        const std::function<void()>& after_board_change) {
    q_core::MakeMove(board, move, make_move_info);
    if (!q_core::WasMoveLegal(board, move)) {
        q_core::UnmakeMove(board, move, make_move_info);
        return false;
    }
    after_board_change();
    evaluator.UpdateOnMove(board, move, make_move_info, evaluator_update_info);
    return true;
}

void Position::MakeNullMove(q_core::coord_t& old_en_passant_coord) {
    q_core::MakeNullMove(board, old_en_passant_coord);
}

void Position::UnmakeNullMove(const q_core::coord_t& old_en_passant_coord) {
    q_core::UnmakeNullMove(board, old_en_passant_coord);
}

bool Position::IsCheck() const { return q_core::IsKingInCheck(board); }

q_core::Board::FENParseStatus Position::MakeFromFEN(const std::string_view& fen) {
    auto res = board.MakeFromFEN(fen);
    if (res != q_core::Board::FENParseStatus::Ok) {
        return res;
    }
    evaluator.StartTrackingBoard(board);
    return q_core::Board::FENParseStatus::Ok;
}

q_eval::score_t Position::GetEvaluatorScore() { return evaluator.Evaluate(board); }

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
