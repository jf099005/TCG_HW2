#ifndef _MCTS_CPP_
#define _MCTS_CPP_
#include<cassert>
#include"mcts_agent.h"
#include"helper.h"
#include"../BoardAnalyze/H/board_analyzer.h"

#include<cmath>
#include <chrono>
using namespace std::chrono;
using namespace std;

// #define DEBUG 1
const int MAX_NON_CAPTURE_MOVE = 31;
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

MCTS_agent::~MCTS_agent(){
    delete [] Nodes;
    delete [] AMAF_Nodes;
    Nodes = nullptr;
    AMAF_Nodes = nullptr;
}


void MCTS_agent::print_node(Node* node){
    debug << "current total simulation: " << this->N <<endl;
    debug <<"node N:" << node->Ntotal <<endl;
    debug <<"avg: " << node->Mean <<", total sq: " << node->sq_score_sum <<", var: " << node->Variance <<endl;
    debug << " depth: " <<node->depth <<endl;
    debug <<"nodes:" <<endl;
    for(int i=0; i<node->Nchild; i++){
        Node* child = get_child(node, i);
        Node* child_amaf = get_AMAF_Node(child);
        debug << '\t' <<"child N: " << child->Ntotal << "/" << child_amaf->Ntotal <<\
                    ", avg: " << child->Mean<<"/" << child_amaf->Mean\
                     <<", total sq: " << child->sq_score_sum<<"/"<< child_amaf->sq_score_sum\
                     <<", var: " << child->Variance << "/" <<child_amaf->Variance <<endl;
        // debug << "\t child of child: " << child->Nchild <<endl;
        // debug << "\t\t move:" << child->move <<endl;
        debug << "\t\t" <<"activated: " << child->is_activated <<endl;
    }

}

void MCTS_agent::reset(Color p_c, Position pos, int remain_moves){
    this->root_pos = pos;
    this->player_color = p_c;
    this->remain_moves = remain_moves;
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
    auto start = high_resolution_clock::now();

    while(true){
        MCTS_iteration();
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        double t = double(duration.count())*microseconds::period::num/microseconds::period::den;
        if(t >= time_constraint){
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
    // update_beta();

    // debug << "\r N:" << root->Ntotal << " / " << get_AMAF_Node(root)->Ntotal <<", " <<  \
    // "beta:" << calculate_beta(this->root) << "\t\t\t\t\t\t\t\t";

    auto PV = search_pv();
    Node* PV_leaf = PV.first;
    Position leaf_pos(PV.second);

    static MOVE_RECORDER moves_record = vector<int>(2*32*32);
    fill(moves_record.begin(), moves_record.end(), 0);
    // moves_record.clear();


    if(leaf_pos.winner() == NO_COLOR and this->PV_remain_moves > 0){
        expand(PV_leaf, leaf_pos);
    }
    else{
        Score avg_score;
        Score all_pieces_score = pieces_score(leaf_pos, leaf_pos.pieces(ALL_PIECES));
        if(leaf_pos.winner() == Mystery or this->PV_remain_moves == 0)
            avg_score = pos_simulate::tie_score;
        else{
            avg_score = leaf_pos.winner() == leaf_pos.due_up() ?\
                        all_pieces_score : -all_pieces_score;
        }
        this->N += n_simulate_expand;
        back_propagation_RAVE(PV_leaf, avg_score, n_simulate_expand, n_simulate_expand);
        // cout<<"reach endgame:" << leaf_pos;
        return false;
    }

    Score total_score = 0.0;
    int total_N_simulation = 0;


    Color child_color = (leaf_pos.due_up()==Red? Black:Red);
    for(int i=0; i<PV_leaf->Nchild; i++){
        Position pos_child(leaf_pos);

        Node* child = get_child(PV_leaf, i);
        Node* child_amaf = get_AMAF_Node(child);
        
        // if(moves_record.find(child->move) == moves_record.end())
        //     moves_record[child->move] = 0;
        // int move_idx = encode_move();

        int child_move_idx = pos_simulate::encode_move(child_color, child->move);
        moves_record[child_move_idx] += n_simulate_expand;
        
        pos_child.do_move( child->move );
        Score avg_simulate_solution;
        
        avg_simulate_solution = simulate_AMAF(pos_child, this->n_simulate_expand, &moves_record, this->PV_remain_moves);
        total_score += -avg_simulate_solution*(this->n_simulate_expand);
        total_N_simulation += this->n_simulate_expand;
        this->N += n_simulate_expand;

        // update_node(child, avg_simulate_solution, this->n_simulate_expand);
        // update_node(child_amaf, avg_simulate_solution, this->n_simulate_expand);
        back_propagation_RAVE(child, avg_simulate_solution, this->n_simulate_expand, 0);
    }

    Score avg_score = total_score / total_N_simulation;
    int leaf_N_amaf = update_AMAF_leaf(PV_leaf, leaf_pos.due_up(), avg_score, &moves_record);

    Score amaf_score = avg_score*leaf_N_amaf;
    back_propagation_RAVE(PV_leaf, avg_score, 0, leaf_N_amaf);


    if(leaf_N_amaf < total_N_simulation){
        debug <<"error at " << leaf_pos;
        debug <<leaf_N_amaf << '/' << total_N_simulation <<endl;
        // for(auto rec: moves_record){
        //     cout<<rec.first << ':' <<rec.second <<endl;
        // }
    }

    assert(leaf_N_amaf >= total_N_simulation);

    Node* cur = PV_leaf;
    // // progressive_cut(cur);
    // // check_node(cur);

    while(cur != nullptr){
        progressive_cut(cur);
        // check_node(cur);
        cur = cur->parent;
    }

    
    return false;
}



pair<Node*, Position> MCTS_agent::search_pv(){
    this->PV_remain_moves = this->remain_moves;
    Node* cur = root;
    Position pos(root_pos);
    while(!cur->can_expand){
        this->PV_remain_moves--;
        assert(PV_remain_moves >= 0);
        Node* selected = this->select_maximum_child(cur);
        Move selected_move = selected->move;
        if( pos.peek_piece_at( selected_move.to() ).type != NO_PIECE ){
            this->PV_remain_moves = MAX_NON_CAPTURE_MOVE;
        }
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

    // return ucb_origin;

    double node_beta = calculate_beta(node);
    // node_beta = 0;
    assert(0 <= node_beta);
    assert(node_beta <= 1);

    return (1-node_beta)*ucb_origin + node_beta * ucb_amaf;
}

Node* MCTS_agent::select_maximum_child(Node* cur_node){//return the index of the child in Nodes
    Node* selected = nullptr; //(cur_node, 0);
    
    assert(cur_node->Nchild > 0);

    double mx_UCB = -inf;//this->UCB_RAVE( selected, cur_node );
    for(int i=0; i< cur_node->Nchild; i++){
        Node* child = get_child(cur_node, i);
        if(!(child->is_activated))
            continue;

        double child_UCB = UCB_RAVE( child, cur_node );
        if( mx_UCB < child_UCB ){
            selected = child;
            mx_UCB = child_UCB;
        }
    }
    if(selected == nullptr){
        print_node(cur_node);
    }
    assert(selected != nullptr);
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
    //return the average score

Score MCTS_agent::simulate_AMAF(Position pos, int n_simulate, MOVE_RECORDER* moves_recorder, int remain_moves){
    Score total_score = 0;
    for(int i=0; i<n_simulate; i++){
        // total_score += pos_simulate(pos);
        total_score += pos_simulate::simulate_and_record(pos, this->player_color, moves_recorder, remain_moves);
    }
    return total_score / n_simulate;
}


//back propagation part
void MCTS_agent::update_node(Node* node, Score avg_score, int n){
    if(n == 0)return;
    // Node &cur_node = Nodes[node_idx];
    node->Ntotal += n;
    node->score_sum += avg_score*n;
    node->sq_score_sum += avg_score*avg_score*n;
    node->sqrtN = sqrt(node->Ntotal);
    node->CsqrtlogN = 
        this->Exploration_coeff * sqrt(log(node->Ntotal));
    node->Mean = (double)(node->score_sum) / node->Ntotal;
    double mean_sq = node->Mean;
    mean_sq *= mean_sq;
    node->Variance = (node->sq_score_sum/node->Ntotal)-mean_sq;
}

void MCTS_agent::cut_child(Node* parent, Node* child){
    assert(child->is_activated);
    Score child_score_sum = child->score_sum;
    while(parent != nullptr){
        child_score_sum = -child_score_sum;
        parent->Ntotal -= child->Ntotal;
        parent->score_sum -= child_score_sum;
        parent->sq_score_sum -= child->sq_score_sum;
        parent->sqrtN = sqrt(parent->Ntotal);
        parent->CsqrtlogN = 
            this->Exploration_coeff * sqrt(log(parent->Ntotal));
        parent->Mean = (double)(parent->score_sum) / parent->Ntotal;
        double mean_sq = parent->Mean;
        mean_sq *= mean_sq;
        parent->Variance = ((double)parent->sq_score_sum/parent->Ntotal - mean_sq);
        parent = parent->parent;
    }
    child->is_activated = false;
}


void MCTS_agent::connect_child(Node* parent, Node* child){
    assert(!(child->is_activated));
    Score child_score_sum = child->score_sum;

    while(parent != nullptr){
        child_score_sum = -child_score_sum;

        parent->Ntotal += child->Ntotal;
        parent->score_sum += child_score_sum;
        parent->sq_score_sum += child->sq_score_sum;
        parent->sqrtN = sqrt(parent->Ntotal);
        parent->CsqrtlogN = 
            this->Exploration_coeff * sqrt(log(parent->Ntotal));
        parent->Mean = (double)(parent->score_sum) / parent->Ntotal;
        double mean_sq = parent->Mean;
        mean_sq *= mean_sq;
        parent->Variance = ((double)parent->sq_score_sum/parent->Ntotal - mean_sq);
        parent = parent->parent;
    }
    child->is_activated = true;
}
//node: original node, not amaf node

void MCTS_agent::progressive_cut(Node* node){
    static double confidence = 2;
    // confidence = confidence * n_simulate_expand;

    Score lower_bound_score = node->Mean;
    lower_bound_score -= confidence * sqrt(node->Variance);
    double origin_var = node->Variance;
    double origin_avg = node->Mean;
    double origin_sq = node->sq_score_sum;
    for(int i=0; i<node->Nchild; i++){
        Node* child = get_child(node, i);
        Score branch_score = -child->Mean;

        branch_score += confidence * sqrt(child->Variance);
        bool cut = false;
        if(branch_score < lower_bound_score){
            if(child->is_activated and child->Ntotal >= 3){
                cut_child(node, child);
                total_cut++;
                cut = true;
            }
        }
        else{
            if(!(child->is_activated)){
                connect_child(node, child);
                total_reconnect++;
                cut = true;
            }
        }
        if(cut)
            lower_bound_score = node->Mean - confidence * sqrt(node->Variance);

    }

    if(node->Variance +1e-9 <= 0){
        cout <<"detect var < 0 :\n";
        cout <<"origin avg:" << origin_avg << endl;
        cout <<"origin var: " << origin_var <<endl;
        cout <<"origin sq sum: " << origin_sq <<endl;
        print_node(node);
    }
    assert(node->Variance+1e-9 >= 0 );
}



bool MCTS_agent::check_node(Node* node){
    int Ntotal = 0;
    for(int i=0; i<node->Nchild; i++){
        Node* child = get_child(node, i);
        if(child->is_activated)
            Ntotal += child->Ntotal;
    }
    if(Ntotal > node->Ntotal or node->Ntotal - Ntotal > this->n_simulate_expand){
        debug << "node N:" << node->Ntotal <<", statistic N:" << Ntotal <<endl;
        print_node(node);
    }
    // assert(node->Ntotal <= get_AMAF_Node(node)->Ntotal);
    assert(Ntotal <= node->Ntotal);
    assert(node->Ntotal - Ntotal <= this->n_simulate_expand);
    return true;
}


int MCTS_agent::update_AMAF_leaf(Node* leaf, Color leaf_color, Score avg_score, MOVE_RECORDER* move_recorder){
    Node* node_amaf = get_AMAF_Node(leaf);
    int node_N_amaf = 0;
    assert(leaf->Nchild > 0);

    assert(leaf_color == Red or leaf_color == Black);
    Color child_color = (leaf_color == Red?Black:Red);

    for(int i=0; i < leaf->Nchild; i++){
        Node* child = get_child(leaf, i);
        int child_move_idx = pos_simulate::encode_move(child_color, child->move);
        if( (*move_recorder)[child_move_idx] > 0 ){
            Node* child_amaf = get_AMAF_Node(child);
            int child_N = (*move_recorder)[child_move_idx];
            assert(child_N > 0 );
            // Score child_score = avg_score*child_N;
            update_node( child_amaf, avg_score, child_N );
            node_N_amaf += (*move_recorder)[child_move_idx];
        }
    }
    N_AMAF += node_N_amaf;
    return node_N_amaf;
}



void MCTS_agent::back_propagation_RAVE(Node* leaf, Score avg_score, int total_n_simulate, int N_amaf){
    Node* cur = leaf;
    Node* cur_amaf = get_AMAF_Node(cur);
    this->update_node(cur, avg_score, total_n_simulate);
    this->update_node(cur_amaf, avg_score, N_amaf);
    while(cur != nullptr){
        avg_score = -avg_score;
        cur = cur->parent;
        if(cur == nullptr)
            return;

        update_node(cur, avg_score, total_n_simulate);
        // progressive_cut(cur);
        // check_node(cur);

        Node* cur_amaf = get_AMAF_Node(cur);
        update_node(cur_amaf, avg_score, N_amaf);

        if(cur->parent == nullptr)
            break;
    }
}



#endif