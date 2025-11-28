#include"endgame_evaluator.h"
#include"board_analyzer.h"
int pieces_score(const Position &pos, Board pieces_location){
    int score = 0;
    for(Square sq: BoardView(pieces_location)){
        score += Piece_Value[ pos.peek_piece_at(sq).type ];
    }
    return score;
}

int endgame_evaluator::evaluate(Position pos, Color side){
    static const int C = 100;
    Color opponent = (side == Black? Red:Black);
    Board player_pieces = pos.pieces(side);
    Board opponent_pieces = pos.pieces(opponent);

    int total_piece_distance = 0;
    int total_piece_score = pieces_score(pos, side) - pieces_score(pos, opponent);

    for(Square sq_p: BoardView(player_pieces)){
        for(Square sq_o: BoardView(opponent_pieces)){
            PieceType player_type = pos.peek_piece_at(sq_p).type;
            PieceType opponent_type = pos.peek_piece_at(sq_o).type;
            if(player_type > opponent_type and 
                                    !(opponent_type > player_type)){
                int piece_dis = distance<Square>(sq_p, sq_o);
                total_piece_distance += piece_dis;
            }
        }
    }

    // assert(total_piece_score > 0);
    
    int score = C*total_piece_score - total_piece_distance;
    if(pos.due_up() != side)
        score = -score;
    return score;
}