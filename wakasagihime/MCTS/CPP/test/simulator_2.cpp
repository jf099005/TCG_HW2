#include"simulator.h"
#include<algorithm>
#include<map>

const int normal_move_score = 5;
const int suicide_move_score = 2;
const int capture_move_score = 10;

map<PieceType, int> Piece_Value = {
    {General, 7},
    {Advisor, 8},
    {Elephant, 6},
    {Chariot, 5},
    {Horse, 3},
    {Cannon, 9},
    {Soldier, 1}
};

int pos_simulate::move_evaluate(const Position& pos, Move move){
    PieceType s = pos.peek_piece_at(move.from()).type;

    PieceType e = pos.peek_piece_at(move.to()).type;

    if( e != NO_PIECE){
        int score = capture_move_score;
        score += Piece_Value[e];
        score -= (Piece_Value[s] >> 1);
        return score;
    }
    
    return normal_move_score;
}

Move pos_simulate::stone_power_greedy_strategy(const Position& pos, MoveList<> &moves){
    static long long move_scores[200];
    static long long total_scores;
    total_scores = 0;
    for(int i=0; i< moves.size(); i++){
        move_scores[i] = move_evaluate(pos, moves[i]);
        total_scores += move_scores[i];
    }

    static long long prefix[200];
    prefix[0] = move_scores[0];
    for (int i = 1; i < moves.size(); i++)
        prefix[i] = prefix[i-1] + move_scores[i];

    long long rd = rng(prefix[ moves.size()-1]) + 1;

    // binary search
    int idx = lower_bound(prefix, prefix + moves.size(), rd) - prefix;
    return moves[idx];
}


Color pos_simulate::early_stop(Position pos){
    return NO_COLOR;
    bool red_win = true;
    bool black_win = true;
    for(Square sq_r: BoardView(pos.pieces(Red))){
        PieceType R = pos.peek_piece_at(sq_r).type;
        for(Square sq_b: BoardView( pos.pieces(Black) )){
            PieceType B = pos.peek_piece_at(sq_b).type;
            red_win &= !(B>R);
            black_win &= !(R>B);
            if(!(red_win or black_win))
                break;
        }
        if(!(red_win or black_win))
            break;
    }


    return red_win? Red : ( black_win? Black : NO_COLOR);
}

Score pos_simulate::simulate(Position pos){
    // return pos.simulate(strategy_random);
    Position copy(pos);
    
    // bool apply_early_stop = early_stop(pos) == NO_COLOR;
    
    Color winner = NO_COLOR;
    while (winner == NO_COLOR) {
        MoveList moves(copy);
        Move rd_move = moves[rng(moves.size())];
        copy.do_move(rd_move);
        winner = copy.winner();
        // if(apply_early_stop and winner == NO_COLOR){
        //     winner = early_stop(pos);
        // }
    }

    if (winner == pos.due_up()) {
        return win_score;
    } else if (winner == Mystery) {
        return tie_score;
    }
    return -win_score;
}