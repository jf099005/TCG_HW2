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
    player_color(p_c), Exploration_coeff(initial_coeff), n_simulate_expand(n_simulate_leaf),
    N(0), N_AMAF(0), maximum_node_idx(0), root_idx(0)
    {
        this->Nodes = new Node[MaxNode];
        // #ifdef AMAF
        //     assert(N_simulate_leaf == 1);
        // #endif
        this->AMAF_Nodes = new Node[MaxNode];
        this->root = create_root( p_c );
        this->root_pos = initial_pos;
    };



void MCTS_agent::reset(Color p_c, Position pos){
    this->root_pos = pos;
    this->player_color = p_c;
    
    this->maximum_node_idx = 0;
    this->root_idx = 0;
    this->root = create_root(p_c);
    this->N = 0;
    this->N_AMAF = 0;
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

    static MOVE_RECORDER moves_record;

    moves_record.clear();


    if(leaf_pos.winner() == NO_COLOR){
        expand(PV_leaf, leaf_pos);
    }
    else{
        Score score;
        if(leaf_pos.winner() == Mystery)score = pos_simulate::tie_score;
        else{
            score = leaf_pos.winner() == leaf_pos.due_up() ?\
                        pos_simulate::win_score : -pos_simulate::win_score;
        }
        this->N += n_simulate_expand;
        back_propregation(PV_leaf, score, n_simulate_expand);        
    }
    
    for(int i=0; i<PV_leaf->Nchild; i++){
        Position pos_child(leaf_pos);

        Node* child = get_child(PV_leaf, i);

        pos_child.do_move( child->move );
        #ifdef AMAF
            Score simulate_solution = simulate_AMAF(pos_child, 1, &moves_record);
            N += n_simulate_expand;
            back_propregation_RAVE(child, simulate_solution, this->n_simulate_expand, &moves_record);
        #else
            Score simulate_solution = simulate(pos_child, this->n_simulate_expand);
            back_propregation(child, simulate_solution, this->n_simulate_expand);
        #endif       
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

long double MCTS_agent::UCB_RAVE(Node* node, Node* parent){
    Node* node_amaf = get_AMAF_Node(node);
    Node* parent_amaf = get_AMAF_Node(parent);

    long double ucb_origin = UCB(node, parent);
    long double ucb_amaf = UCB(node_amaf, parent_amaf);

    return (1-this->beta)*ucb_origin + this->beta * ucb_amaf;
}

Node* MCTS_agent::select_maximum_child(Node* cur_node){//return the index of the child in Nodes
    Node* selected = get_child(cur_node, 0);
    
    assert(cur_node->Nchild > 0);

    double mx_UCB = this->UCB_RAVE( selected, cur_node );
    for(int i=1; i< cur_node->Nchild; i++){
        Node* child = get_child(cur_node, i);
        double child_UCB = UCB_RAVE( child, cur_node );
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

Score MCTS_agent::simulate(Position pos, int n_simulate){
    Score total_score = 0;
    while(n_simulate--){
        // total_score += pos_simulate(pos);
        total_score += pos_simulate::simulate(pos);
    }
    return total_score;
}

Score MCTS_agent::simulate_AMAF(Position pos, int n_simulate, MOVE_RECORDER* moves_recorder){
    Score total_score = 0;
    while(n_simulate--){
        // total_score += pos_simulate(pos);
        total_score += pos_simulate::simulate_and_record(pos, moves_recorder);
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

//node: original node, not amaf node



void MCTS_agent::update_node_AMAF(Node* node, Score score, int n_simulate, MOVE_RECORDER* move_recorder){
    Node* node_amaf = get_AMAF_Node(node);
    int node_N_amaf = 0;

    for(int i=0; i < node->Nchild; i++){
        Node* child = get_child(node, i);
        if( move_recorder->find( child->move ) != move_recorder->end() ){
            Node* child_amaf = get_AMAF_Node(child);
            update_node( child_amaf, -score, (*move_recorder)[child->move] );
            node_N_amaf += (*move_recorder)[child->move];
        }
    }

    update_node(node_amaf, score, node_N_amaf);

    N_AMAF += node_N_amaf;
}

void MCTS_agent::update_node_RAVE(Node* node, Score score, int n_simulate, MOVE_RECORDER* move_recorder){
    update_node(node, score, n_simulate);
    update_node_AMAF(node, score, n_simulate, move_recorder);
}

void MCTS_agent::back_propregation(Node* leaf, Score score, int n_simulate){
    Node* cur = leaf;
    this->update_node(cur, score, n_simulate);
    while(cur != nullptr){
        score = -score;
        cur = cur->parent;
        this->update_node(cur, score, n_simulate);
        if(cur->parent == nullptr)
            break;
    }
}



void MCTS_agent::back_propregation_RAVE(Node* leaf, Score score, int n_simulate, MOVE_RECORDER* move_recorder){
    Node* cur = leaf;
    this->update_node(cur, score, n_simulate);
    while(cur != nullptr){
        score = -score;
        cur = cur->parent;
        this->update_node_RAVE(cur, score, n_simulate, move_recorder);
        if(cur->parent == nullptr)
            break;
    }
}



#endif