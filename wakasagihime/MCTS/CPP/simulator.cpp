#include"simulator.h"
#include<algorithm>
#include<map>
#include"../BoardAnalyze/H/board_analyzer.h"
const int normal_move_score = 1;
// const int suicide_move_score = 2;
const int capture_move_score = 10;

int pos_simulate::move_evaluate(const Position& pos, Move move){
    PieceType s = pos.peek_piece_at(move.from()).type;

    PieceType e = pos.peek_piece_at(move.to()).type;

    if( e != NO_PIECE){
        int score = capture_move_score;
        score += Piece_Value[e];
        if(e == Cannon){
            score -= (Piece_Value[s] >> 1);
        }
        else{
            score += (Piece_Value[s] >> 1);
        }
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

int pos_simulate::encode_move(Color side, Move mv){
    int idx = mv.from()*32 + mv.to();
    idx += (side == Red)*1024;
    return idx;
}

inline Color is_endgame(Position pos){

    // return NO_COLOR;
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


int max_piece_score(Position pos, Board pieces){
    int mx = 0;
    int score = 0;
    for(Square sq: BoardView(pieces)){
        PieceType pt = pos.peek_piece_at(sq).type;
        if(pt != Cannon){
            if(Piece_Value[pt] == mx)
                score += mx;
            if(Piece_Value[pt] > mx){
                mx = Piece_Value[pt];
                score = mx;
            }
            // mx = max(mx, Piece_Value[pt]);
        }
    }
    return mx;
}

Score pos_simulate::simulate_and_record(Position pos, Color player_color, MOVE_RECORDER* move_recorder, int remain_moves){
    Position copy(pos);
    Color winner = copy.winner();

    // bool apply_early_stop = (early_stop(pos) == NO_COLOR);

    while (winner == NO_COLOR) {
        MoveList moves(copy);
        remain_moves--;
        // Move move = moves[rng(moves.size())];
        Move move = stone_power_greedy_strategy(copy, moves);
        if(copy.peek_piece_at( move.to() ).type != NO_PIECE){
            remain_moves = 30;
        }

        int move_idx = encode_move(copy.due_up(), move);

        copy.do_move(move);

        (*move_recorder)[move_idx]++;
        // if(move_recorder->find(move) == move_recorder->end())
        //     (*move_recorder)[move] = 1;

        // (*move_recorder)[move] += 1;

        winner = copy.winner();
        if(remain_moves == 0){
            winner = Mystery;
        }
        // if(winner == NO_COLOR and apply_early_stop)
        //     winner = early_stop(copy);
    }

    Score board_score = max_piece_score(copy, copy.pieces());

    if (winner == pos.due_up()) {
        return board_score;
    } else if (winner == Mystery) {
        if(pos.due_up() != player_color)
            return tie_score;
        else{
            return -tie_score;
        }
    }
    return -board_score;

    return 0;
}

