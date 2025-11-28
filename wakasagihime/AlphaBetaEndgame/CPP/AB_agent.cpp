#include"AB_agent.h"

int AlphaBetaAgent::Negamax(Position pos, int alpha, int beta, int depth){
    if(depth == 0){
        return endgame_evaluator::evaluate(pos, pos.due_up());
    }

    int opt = -score_mx;
    MoveList<> nx_moves(pos);

    for(Move nx_move: nx_moves){
        Position pos_nx(pos);
        pos_nx.do_move(nx_move);
        int v = Negamax(pos_nx, -beta, -alpha, depth-1);
        if(v >= beta){
            return v;
        }
        opt = max(opt, v);
        alpha = max(alpha, v);
    }


}