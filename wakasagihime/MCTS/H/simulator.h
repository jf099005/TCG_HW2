#include"node.h"
#ifndef __SIMULATOR__
#define __SIMULATOR__

#include<map>

typedef long double Score;
// typedef map<Move, int> MOVE_RECORDER;
typedef vector< int > MOVE_RECORDER;

namespace pos_simulate{

    const Score win_score = 10;//lose = -1
    const Score tie_score = 4;

    int move_evaluate(const Position& pos, Move move);
    int encode_move(Color side, Move move);
    Move stone_power_greedy_strategy(const Position& pos, MoveList<> &moves);
    // Score simulate(Position pos);
    Score simulate_and_record(Position pos, Color player_color, MOVE_RECORDER* moves_record, int remain_moves = 30);

}

#endif