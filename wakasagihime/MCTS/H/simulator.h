#include"node.h"
#ifndef SIMULATOR
#define SIMULATOR 1

typedef long double Score;


namespace pos_simulate{

    const Score win_score = 5;//lose = -1
    const Score tie_score = -2;

    int move_evaluate(const Position& pos, Move move);
    Move stone_power_greedy_strategy(const Position& pos, MoveList<> &moves);
    Score simulate(Position pos);

}

#endif