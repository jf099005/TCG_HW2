#include"node.h"
#include"simulator.h"
#include<map>
#ifndef _MCTS_H_
#define _MCTS_H_

#define AMAF 1
#define DEFAULT_N_SIMULATE 1

//Red for positive score, Black for negative score

class MCTS_agent{
    public:
        // MCTS_agent() = default;

        MCTS_agent(Color p_c, Position initial_pos, double initial_coeff = 1.0, int n_simulate_leaf = 5);
        ~MCTS_agent();
        void reset(Color p_c, Position initial_pos, int remain_moves = 30);
        void MCTS_simulate(int N_simulate, double time_constraint = 5.0);
        bool MCTS_iteration();//return true if early-stop
        Move opt_solution();
        
        Color player_color;
        // when expand, simulate n_simulate_expand times for each new leaves 
        int n_simulate_expand;
        int remain_moves;


    
    //private:
        //node operations
            Position root_pos;
            Node *Nodes;
            Node *AMAF_Nodes;
            Node* root;
            int maximum_node_idx;
            int estimate_total_simulate;

            void estimate(int n){
                this->estimate_total_simulate = n*30;
            }

            Node* create_root(const Color &cur_color){
                Node root =  Node(cur_color,  maximum_node_idx);
                Nodes[maximum_node_idx] = root;

                #ifdef AMAF
                    Node AMAF_root = Node(cur_color, maximum_node_idx);
                    AMAF_Nodes[maximum_node_idx] = AMAF_root;
                #endif

                return Nodes + (maximum_node_idx++);
            }

            Node* create_sucessor(Node* parent, Move child_move){
                Nodes[maximum_node_idx] = Node(maximum_node_idx, parent, child_move);
                Node* AMAF_parent = get_AMAF_Node(parent);
                AMAF_Nodes[maximum_node_idx] = Node(maximum_node_idx, AMAF_parent, child_move);
                return Nodes + (maximum_node_idx++);
            }

            inline Node* get_child(Node* node, int child_idx){
                assert(node->Nchild > child_idx);
                return Nodes + node->c_id[child_idx];
            }

            inline Node* get_AMAF_Node(Node* node){
                return AMAF_Nodes + node->id;
            }


        //MCTS ELEMENTS
            int root_idx;
            double Exploration_coeff;
            //return the index and the position of the leaf in PV
            int PV_remain_moves;
            pair<Node*, Position> search_pv();

            //find the maximum children
            Node* select_maximum_child(Node* node);
            // Node select_maximum_child_idx(int node_idx);

            void expand(Node* node, Position node_pos);

            //return the result of simulate in a given number of simulation
            //w.r.t. the player of pos
            Score simulate_AMAF(Position pos, int n_simulate, MOVE_RECORDER* moves_record, int remain_moves);

            //used for back_propagation
            void update_node(Node* node, Score score, int n_simulate);//w: number of winning, n: total number of simulation


            void cut_child(Node* parent, Node* child);
            void connect_child(Node* parent, Node* child);

            void progressive_cut(Node* node);
            //node: original node, not amaf node
            //return the amaf n_simulation
            int update_AMAF_leaf(Node* node, Color leaf_color, Score score, MOVE_RECORDER* recorder);//w: number of winning, n: total number of simulation
            
            void back_propagation_RAVE(
                Node* leaf, \
                Score avg_score,\
                int n_simulate, int n_AMAF
            );

        //utils for expansion
            //UCB-score of node_id w.r.t. its parent color
            long double UCB_RAVE(Node* node, Node* parent);
            long double UCB(Node* node, Node* parent);
            
            double beta = 1.0;
            int N, N_AMAF;
            double calculate_beta(Node* node){
                const static double b = 0.0001;
                Node* amaf_node = get_AMAF_Node(node);
                int node_N = node->Ntotal;
                int node_N_amaf = amaf_node->Ntotal;
                if(node_N == 0)
                    return 0.9;
                beta =  node_N_amaf / (node_N + node_N_amaf + 4*b*node_N*node_N_amaf);
                return beta;
            }

            void update_beta(){
                const static double b = 0.01;
                // this->beta = (this->N_AMAF)\
                //              /(this->N + this->N_AMAF + 4*b*this->N*this->N_AMAF);
                // this->beta = 0;
                this->beta = 1 - min(1.0, double(N)/100000.0);
            };
};

#endif