#include"node.h"
#include"simulator.h"
#ifndef _MCTS_H_
#define _MCTS_H_

#define DEFAULT_N_SIMULATE 1

//Red for positive score, Black for negative score

class MCTS_agent{
    public:
        // MCTS_agent() = default;

        MCTS_agent(Color p_c, Position initial_pos, double initial_coeff = 1.0, int n_simulate_leaf = 5);
        void reset(Color p_c, Position initial_pos);
        void MCTS_simulatie(int N_simulate, double time_constraint = 5.0);
        bool MCTS_iteration();//return true if early-stop
        Move opt_solution();
        
        Color player_color;
        // when expand, simulate n_simulate_expand times for each new leaves 
        int n_simulate_expand;


    
    //private:
        //node operations
            Position root_pos;
            Node *Nodes;
            Node* root;
            int maximum_node_idx;

            Node* create_root(const Color &cur_color){
                Node root =  Node(cur_color,  maximum_node_idx);
                Nodes[maximum_node_idx] = root;
                return Nodes + (maximum_node_idx++);
            }

            Node* create_sucessor(Node* parent, Move child_move){
                Nodes[maximum_node_idx] = Node(maximum_node_idx, parent, child_move);
                return Nodes + (maximum_node_idx++);
            }


        //MCTS ELEMENTS
            int root_idx;
            double Exploration_coeff;
            //return the index and the position of the leaf in PV
            pair<Node*, Position> search_pv();

            inline Node* get_child(Node* node, int child_idx){
                assert(node->Nchild > child_idx);
                return Nodes + node->c_id[child_idx];
            }

            //find the maximum children
            Node* select_maximum_child(Node* node);
            // Node select_maximum_child_idx(int node_idx);

            void expand(Node* node, Position node_pos);

            //return the result of simulate in a given number of simulation
            //w.r.t. the player of pos
            Score simulate(Position pos, int n_simulate);
            Score pos_simulate(Position pos);

            //used for back_propregation
            void update_node(Node* node, Score score, int n_simulate);//w: number of winning, n: total number of simulation
            void back_propregation(Node* leaf, Score score, int n_simulate);

        //utils for expansion
            //UCB-score of node_id w.r.t. its parent color
            long double UCB(Node* node, Node* parent);


};

#endif