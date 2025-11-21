#ifndef _MCTS_CPP_
#define _MCTS_CPP_
#include<cassert>
#include"mcts_agent.h"
#include"helper.h"
#include<cmath>
using namespace std;

// #define DEBUG 1
const int N_threshold = 10000;
const long double inf = 1e9;
const double win_score = 1;//lose = -1
const double tie_score = -0.05;

MCTS_agent::MCTS_agent(Color p_c, Position initial_pos, double initial_coeff, int n_simulate_leaf):
        player_color(p_c), Exploration_coeff(initial_coeff), n_simulate_expand(n_simulate_leaf)
    {
        this->Nodes = new Node[MaxNode];

        this->maximum_node_idx = 0;
        this->root_idx = create_root( p_c );
        this->root_pos = initial_pos;
    };

void MCTS_agent::reset(Color p_c, Position pos){
    this->root_pos = pos;
    this->player_color = p_c;
    
    this->maximum_node_idx = 0;
    this->root_idx = create_root(p_c);
}

Move MCTS_agent::opt_solution(double time_constraint, int N_simulate){
    MCTS_simulatie(time_constraint, N_simulate);
    int select_idx = select_maximum_child_idx(root_idx);
    return Nodes[root_idx].c_move[select_idx];
}

void MCTS_agent::MCTS_simulatie(int N_simulate, double time_constraint){
    while(N_simulate--){
        if( MCTS_iteration() ){
            // cout<<"remaining: " << N_simulate <<endl;
            return;
        }
    }
}



bool MCTS_agent::MCTS_iteration(){    
    #ifdef DEBUG
        cout <<"simulate iteration" <<endl;
        cout<<"initial state:" << root_pos <<endl;

        cout<<"start to search PV\n";
    #endif
    auto PV = search_pv();
    int PV_leaf_idx = PV.first;
    Position leaf_pos(PV.second);

    if(leaf_pos.winner() != NO_COLOR){
        return true;
    }

    #ifdef DEBUG
        cout<<"PV:" <<leaf_pos<<endl;
    #endif

    expand(PV_leaf_idx, leaf_pos);

    #ifdef DEBUG
        cout << "finish expand" <<endl;
    #endif

    for(int i=0; i<Nodes[PV_leaf_idx].Nchild; i++){
        Position pos_child(leaf_pos);
        pos_child.do_move( Nodes[PV_leaf_idx].c_move[i] );

        int child_idx = Nodes[PV_leaf_idx].c_id[i];
        
        Score simulate_solution = simulate(pos_child, this->n_simulate_expand);
        back_propregation(child_idx, simulate_solution, this->n_simulate_expand);
    }

    #ifdef DEBUG
        cout << "finish all simulations" <<endl;
    #endif

    return false;
}

pair<int, Position> MCTS_agent::search_pv(){
    int cur_node_idx = root_idx;
    Position pos(root_pos);
    while(!Nodes[cur_node_idx].can_expand){
        int selected = this->select_maximum_child_idx(cur_node_idx);
        
        pos.do_move( Nodes[cur_node_idx].c_move[selected] );
        cur_node_idx = Nodes[cur_node_idx].c_id[selected];
        // #ifdef DEBUG
        //     cout << "current idx " << cur_node_idx <<endl;
        //     cout <<"\ncurrent state:" << pos <<endl;
        //     cout << "expandable: " << Nodes[cur_node_idx].can_expand <<endl;
        //     cout <<"next moves: " << Nodes[cur_node_idx].Nchild <<endl;
        //     for(int j=0; j<Nodes[cur_node_idx].Nchild; j++){
        //         cout <<"idx: " << Nodes[cur_node_idx].c_id[j]<<endl;
        //         cout << Nodes[cur_node_idx].c_move[j];
        //     }
        //     cout<<"\n";
        // #endif
    }
    return {cur_node_idx, pos};
}

long double MCTS_agent::UCB(int child_idx, int parent_idx){
    double sqrt_Ni = Nodes[child_idx].sqrtN;
    if(Nodes[child_idx].Ntotal >= N_threshold){
        return -Nodes[child_idx].Mean;
    }
    if(sqrt_Ni == 0){
        return -inf;
    }
    
    double score_i = -Nodes[child_idx].Mean; // take negative for Negamax search
    double csqrt_log_N = Nodes[parent_idx].CsqrtlogN;
    return score_i + csqrt_log_N / sqrt_Ni;
}

int MCTS_agent::select_maximum_child_idx(int node_idx){
    int selected = 0;
    
    assert(Nodes[node_idx].Nchild > 0);

    double mx_UCB = this->UCB( Nodes[node_idx].c_id[0], node_idx );
    for(int i=1; i< Nodes[node_idx].Nchild; i++){
        int child_id = Nodes[node_idx].c_id[i];
        double child_UCB = UCB( child_id, node_idx );
        if( mx_UCB < child_UCB ){
            selected = i;
        }
    }
    return selected;
}



//version 0: generate all next-moves
void MCTS_agent::expand(int node_idx, Position node_pos){
    MoveList nx_moves( node_pos );

    // #ifdef DEBUG
    //     cout << "expand node " << node_idx << node_pos <<endl;
    //     cout<<"--\n";
    //     cout<<"nx moves:";
    //     for(Move mv: nx_moves)cout<<mv;
    //     cout<<"\n";
    // #endif

    for(int i=0; i< nx_moves.size(); i++){

        Nodes[node_idx].c_move[i] = nx_moves[i];
        Nodes[node_idx].c_id[i] = create_sucessor(node_idx);
        
        Position child_pos(node_pos);
        child_pos.do_move(Nodes[node_idx].c_move[i]);

        // #ifdef DEBUG
        //     cout <<"expand node, idx:" << Nodes[node_idx].c_id[i] <<endl;
        //     cout<<"move:" << Nodes[node_idx].c_move[i] <<"/" << nx_moves[i]<<endl;
        //     cout << nx ;
        
        //     cout << "-----\n";
        // #endif
    }
    Nodes[node_idx].Nchild = nx_moves.size();
    Nodes[node_idx].can_expand = 0;

    // #ifdef DEBUG
    //     cout <<"successors:" <<endl;
    //     for(int i=0; i<Nodes[node_idx].Nchild; i++){
    //         cout << "idx:" << Nodes[node_idx].c_id[i] <<", move:" <<  Nodes[node_idx].c_move[i];
    //     }
    //     cout<<"--\n";
    // #endif
}

//simulate part
    //version 0: default random simulate
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
    this->update_node(current_idx, score, n_simulate);
    while(true){
        score = -score;
        current_idx = Nodes[current_idx].p_id;
        this->update_node(current_idx, score, n_simulate);
        if(Nodes[current_idx].p_id == current_idx)
            break;
    }
}



#endif