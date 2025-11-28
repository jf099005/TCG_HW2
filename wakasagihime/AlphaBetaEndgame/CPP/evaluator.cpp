#include"evaluator.h"

int pieces_score(Position pos, Color side){
    int score = 0;
     
}

int endgame_evaluator::evaluate(Position pos, Color side){
    static const int C = 10;
    Color opponent = (side == Black? Red:Black);
    Board player_pieces = pos.pieces(side);
    Board opponent_pieces = pos.pieces(opponent);
    int total_piece_distance = 0;
    int total_piece_score = 0;

    for(Square sq_p: BoardView(player_pieces)){
        for(Square sq_o: BoardView(opponent_pieces)){
            if(pos.peek_piece_at(sq_p).type > pos.peek_piece_at(sq_o).type){
                int piece_dis = distance<Square>(sq_p, sq_o);
                total_piece_distance += piece_dis;
            }
        }
    }

    assert(total_piece_score > 0);
    
    return C*total_piece_score - total_piece_distance;
}