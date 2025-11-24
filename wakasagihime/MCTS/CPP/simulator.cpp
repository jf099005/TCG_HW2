#include"simulator.h"
#include<algorithm>
#include<map>

const int normal_move_score = 1;
// const int suicide_move_score = 2;
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


//stable version
// Score pos_simulate::simulate(Position pos){
//     Position copy(pos);
//     while (copy.winner() == NO_COLOR) {
//         MoveList moves(copy);
//         Move rd_move = moves[rng(moves.size())];
//         copy.do_move(rd_move);
//     }
//     if (copy.winner() == pos.due_up()) {
//         return win_score;
//     } else if (copy.winner() == Mystery) {
//         return tie_score;
//     }
//     return -win_score;

//     return 0;
// }

Score pos_simulate::simulate(Position pos){
    Position copy(pos);
    Color winner = copy.winner();

    // bool apply_early_stop = (early_stop(pos) == NO_COLOR);

    while (winner == NO_COLOR) {
        MoveList moves(copy);
        // Move move = moves[rng(moves.size())];
        Move move = stone_power_greedy_strategy(copy, moves);
        copy.do_move(move);
        winner = copy.winner();
        // if(winner == NO_COLOR and apply_early_stop)
        //     winner = early_stop(copy);
    }
    if (copy.winner() == pos.due_up()) {
        return win_score;
    } else if (copy.winner() == Mystery) {
        return tie_score;
    }
    return -win_score;

    return 0;
}


Score pos_simulate::simulate_and_record(Position pos, MOVE_RECORDER* move_recorder){
    Position copy(pos);
    Color winner = copy.winner();

    // bool apply_early_stop = (early_stop(pos) == NO_COLOR);

    while (winner == NO_COLOR) {
        MoveList moves(copy);
        // Move move = moves[rng(moves.size())];
        Move move = stone_power_greedy_strategy(copy, moves);
        copy.do_move(move);
        if(move_recorder->find(move) == move_recorder->end())
            (*move_recorder)[move] = 1;

        (*move_recorder)[move] += 1;

        winner = copy.winner();
        // if(winner == NO_COLOR and apply_early_stop)
        //     winner = early_stop(copy);
    }
    if (copy.winner() == pos.due_up()) {
        return win_score;
    } else if (copy.winner() == Mystery) {
        return tie_score;
    }
    return -win_score;

    return 0;
}

