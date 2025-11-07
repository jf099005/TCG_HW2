#ifndef _MCTS_CPP_
#define _MCTS_CPP_

#include"mcts_agent.h"
#include"helper.h"
#include<cmath>
using namespace std;

const long double inf = 1e9;

MCTS_agent::MCTS_agent(Color p_c, Position initial_pos, double initial_coeff, int n_simulate_leaf):
        player_color(p_c), Exploration_coeff(initial_coeff), n_simulate_expand(n_simulate_leaf)
    {
        this->maximum_node_idx = 0;
        this->root_idx = 0;
        this->Nodes = new Node[MaxNode];
        this->Nodes[ this->root_idx ] = Node(p_c, this->root_idx);
    };

Move MCTS_agent::opt_solution(double time_constraint, int N_simulate){
    MCTS_simulatie(time_constraint, N_simulate);
    int select_idx = select_maximum(root_idx);
    return Nodes[root_idx].c_move[select_idx];
}

void MCTS_agent::MCTS_simulatie(double time_constraint, int N_simulate){
    while(N_simulate--){
        MCTS_iteration();
    }
}

void MCTS_agent::MCTS_iteration(){
    Position leaf_pos(root_pos);
    int PV_leaf_idx = search_pv( leaf_pos );
    expand(PV_leaf_idx, leaf_pos);
    for(int i=0; i<Nodes[PV_leaf_idx].Nchild; i++){
        Position pos_child(leaf_pos);
        pos_child.do_move( Nodes[PV_leaf_idx].c_move[i] );

        int child_idx = Nodes[PV_leaf_idx].c_id[i];
        
        Score simulate_solution = simulate(pos_child, this->n_simulate_expand);
        back_propregation(child_idx, simulate_solution, this->n_simulate_expand);
    }
}

int MCTS_agent::search_pv(Position &pos){
    int cur_node_idx = root_idx;
    while(!Nodes[cur_node_idx].can_expand){
        int selected = this->select_maximum(cur_node_idx);
        pos.do_move( Nodes[cur_node_idx].c_move[selected] );
        cur_node_idx = Nodes[cur_node_idx].c_id[selected];
    }
    return cur_node_idx;
}

long double MCTS_agent::UCB(int child_idx, int parent_idx){
    double sqrt_Ni = Nodes[child_idx].sqrtN;
    
    if(sqrt_Ni == 0){
        return -inf;
    }
    
    double score_i = -Nodes[child_idx].Mean; // take negative for Negamax search
    double csqrt_log_N = Nodes[parent_idx].CsqrtlogN;
    return score_i + csqrt_log_N / sqrt_Ni;
}

int MCTS_agent::select_maximum(int node_idx){
    int selected = Nodes[node_idx].c_id[0];
    double mx_UCB = this->UCB( selected, node_idx );
    for(int i=1; i< Nodes[node_idx].Nchild; i++){
        int child_id = Nodes[node_idx].c_id[i];
        double child_UCB = UCB( child_id, node_idx );
        if( mx_UCB < child_UCB ){
            selected = child_id;
        }
    }
    return selected;
}


//version 0: generate all next-moves
void MCTS_agent::expand(int node_idx, Position node_pos){
    MoveList nx_moves( node_pos );
    for(int i=0; i< nx_moves.size(); i++){
        Nodes[node_idx].c_move[i] = nx_moves[i];
        Nodes[node_idx].c_id[i] = create_sucessor(node_idx);
    }

    Nodes[node_idx].Nchild = nx_moves.size();
    Nodes[node_idx].can_expand = 1;
}

//simulate part
    //version 0: default random simulate

// Move strategy_random(MoveList<> &moves){
//     return moves[rng(moves.size())];
// }
int MCTS_agent::simulate(Position pos, int n_simulate){
    int total_score = 0;
    while(n_simulate--){
        total_score += pos.simulate(strategy_random);
    }
    return total_score;
}


//back propregation part
void MCTS_agent::update_node(int node_idx, Score w, int n){
    Node &cur_node = Nodes[node_idx];
    cur_node.Ntotal += n;
    cur_node.score_sum += w;
    cur_node.sq_score_sum += w*w;
    cur_node.sqrtN = sqrt(cur_node.Ntotal);
    cur_node.CsqrtlogN = 
        this->Exploration_coeff * sqrt(log(cur_node.Ntotal));
    cur_node.Mean = (double)(cur_node.score_sum) / cur_node.Ntotal;
    double mean_sq = cur_node.Mean;
    mean_sq *= mean_sq;
    cur_node.Variance = ((double)cur_node.sq_score_sum/cur_node.Ntotal - mean_sq);
}

void MCTS_agent::back_propregation(int leaf_idx, Score score, int n_simulate){
    int current_idx = leaf_idx;
    while(Nodes[current_idx].p_id != current_idx){
        this->update_node(current_idx, score, n_simulate);
        current_idx = Nodes[current_idx].p_id;
    }
}



#endif