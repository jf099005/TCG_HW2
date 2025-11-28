#include"AB_agent.h"

int AlphaBetaEndgameSolver::Negamax(Position pos, int depth, int alpha, int beta){
    if(pos.winner() != NO_COLOR){
        if(pos.winner() == solver_color){
            return score_mx;
        }
        return -score_mx;
    }
    
    if(depth == 0){
        return endgame_evaluator::evaluate(pos, pos.due_up());
    }

    int opt = -score_mx;
    MoveList<> nx_moves(pos);

    for(Move nx_move: nx_moves){
        Position pos_nx(pos);
        pos_nx.do_move(nx_move);
        int v = Negamax(pos_nx, depth-1, -opt, -alpha);
        if(v >= beta){
            return v;
        }
        if(v > opt){
            opt = v;
            if(depth == depth_limit){
                opt_solution = nx_move;
            }
        }
        // opt = max(opt, v);
    }
    return opt;

}