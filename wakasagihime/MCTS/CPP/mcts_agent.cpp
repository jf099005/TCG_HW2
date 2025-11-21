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
MCTS_agent::MCTS_agent(Color p_c, Position initial_pos, double initial_coeff, int n_simulate_leaf):
        player_color(p_c), Exploration_coeff(initial_coeff), n_simulate_expand(n_simulate_leaf)
    {
        this->Nodes = new Node[MaxNode];

        this->maximum_node_idx = 0;
        this->root_idx = 0;
        this->root = create_root( p_c );
        this->root_pos = initial_pos;
    };



void MCTS_agent::reset(Color p_c, Position pos){
    this->root_pos = pos;
    this->player_color = p_c;
    
    this->maximum_node_idx = 0;
    this->root_idx = 0;
    this->root = create_root(p_c);
}

Move MCTS_agent::opt_solution(){
    // int select_idx = select_maximum_child_idx(root_idx);
    Node* cur = this->root;
    assert(cur->Nchild > 0);
    Node* selected = get_child(root, 0);
    double mx = -1000000;
    for(int i=0; i< cur->Nchild; i++){
        Node* child = get_child(root, i);

        double score_i = -child->Mean; // take negative for Negamax search

        if( mx < score_i ){
            selected = child;
            mx = score_i;
        }
    }
    return selected->move;


    // return Nodes[root_idx].c_move[select_idx];
}

void MCTS_agent::MCTS_simulate(int N_simulate, double time_constraint){
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
    Node* PV_leaf = PV.first;
    Position leaf_pos(PV.second);

    if(leaf_pos.winner() == NO_COLOR){
        expand(PV_leaf, leaf_pos);
    }

    #ifdef DEBUG
        cout<<"PV:" <<leaf_pos<<endl;
    #endif

    #ifdef DEBUG
        cout << "finish expand" <<endl;
    #endif

    for(int i=0; i<PV_leaf->Nchild; i++){
        Position pos_child(leaf_pos);

        Node* child = get_child(PV_leaf, i);

        pos_child.do_move( child->move );

        
        Score simulate_solution = simulate(pos_child, this->n_simulate_expand);
        back_propregation(child, simulate_solution, this->n_simulate_expand);
    }

    #ifdef DEBUG
        cout << "finish all simulations" <<endl;
    #endif

    return false;
}

pair<Node*, Position> MCTS_agent::search_pv(){
    Node* cur = root;
    Position pos(root_pos);
    while(!cur->can_expand){
        Node* selected = this->select_maximum_child(cur);
        
        pos.do_move( selected->move );
        cur = selected;
    }
    return {cur, pos};
}

long double MCTS_agent::UCB(Node* node, Node* parent){
    double sqrt_Ni = node->sqrtN;
    // if(node->Ntotal >= N_threshold){
    //     return -node->Mean;
    // }
    if(sqrt_Ni == 0){
        return inf;
    }
    
    double score_i = -node->Mean; // take negative for Negamax search
    double csqrt_log_N = parent->CsqrtlogN;
    return score_i + csqrt_log_N / sqrt_Ni;
}

Node* MCTS_agent::select_maximum_child(Node* cur_node){//return the index of the child in Nodes
    Node* selected = get_child(cur_node, 0);
    
    assert(cur_node->Nchild > 0);

    double mx_UCB = this->UCB( selected, cur_node );
    for(int i=1; i< cur_node->Nchild; i++){
        Node* child = get_child(cur_node, i);
        double child_UCB = UCB( child, cur_node );
        if( mx_UCB < child_UCB ){
            selected = child;
            mx_UCB = child_UCB;
        }
    }
    return selected;
}



//version 0: generate all next-moves
void MCTS_agent::expand(Node* node, Position node_pos){
    MoveList nx_moves( node_pos );

    for(int i=0; i< nx_moves.size(); i++){
        Node* child = create_sucessor(node, nx_moves[i]);
        node->c_id[i] = child->id;
    }
    node->Nchild = nx_moves.size();
    node->can_expand = 0;
}

//simulate part
    //version 0: default random simulate

// Score MCTS_agent::pos_simulate(Position pos){
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
// }

int MCTS_agent::simulate(Position pos, int n_simulate){
    int total_score = 0;
    while(n_simulate--){
        // total_score += pos_simulate(pos);
        total_score += pos_simulate::simulate(pos);
    }
    return total_score;
}


//back propregation part
void MCTS_agent::update_node(Node* node, Score w, int n){
    // Node &cur_node = Nodes[node_idx];
    node->Ntotal += n;
    node->score_sum += w;
    node->sq_score_sum += w*w;
    node->sqrtN = sqrt(node->Ntotal);
    node->CsqrtlogN = 
        this->Exploration_coeff * sqrt(log(node->Ntotal));
    node->Mean = (double)(node->score_sum) / node->Ntotal;
    double mean_sq = node->Mean;
    mean_sq *= mean_sq;
    node->Variance = ((double)node->sq_score_sum/node->Ntotal - mean_sq);
}

void MCTS_agent::back_propregation(Node* leaf, Score score, int n_simulate){
    Node* cur = leaf;
    this->update_node(cur, score, n_simulate);
    while(true){
        score = -score;
        cur = cur->parent;
        this->update_node(cur, score, n_simulate);
        if(cur->parent == nullptr)
            break;
    }
}



#endif