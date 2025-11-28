#include"node.h"
#include"evaluator.h"
#ifndef __ALPHABETA__
#define __ALPHABETA__ 1

const int score_mx = 1e9;

class AlphaBetaAgent{
    public:
        AlphaBetaAgent(){}
        
        int Negamax(Position pos, int alpha = -score_mx, int beta = score_mx, int depth);

};

#endif