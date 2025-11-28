#include"AB_agent.h"

const static int move_penalty = 10000;

int AlphaBetaEndgameSolver::Negamax(Position pos, int depth, int alpha, int beta){
    if(pos.winner() != NO_COLOR){
        // if(pos.winner() == pos.due_up()){
        //     return score_mx;
        // }
        // if(pos.winner() == Mystery and pos.due_up() != solver_color){
        //     return score_mx;
        // }
        if(pos.winner() == Mystery)
            return 0;
        return -score_mx;
    }
    
    if(depth == 0){
        return endgame_evaluator::evaluate(pos, solver_color);
    }

    int opt = -score_mx;
    MoveList<> nx_moves(pos);

    for(Move nx_move: nx_moves){
        Position pos_nx(pos);

        pos_nx.do_move(nx_move);
        if(pos_nx.winner() != NO_COLOR){
            return score_mx;
        }
        int v = -Negamax(pos_nx, depth-1, -beta, -opt) - move_penalty;
        if(depth == depth_limit){
            debug << nx_move <<":" << v <<endl;
        }
        if(v > opt){
            opt = v;
            if(depth == depth_limit){
                opt_solution = nx_move;
            }
        }

        if(v >= beta){
            return v;
        }
        // opt = max(opt, v);
    }
    return opt;

}