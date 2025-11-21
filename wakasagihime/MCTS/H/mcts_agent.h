#include"node.h"
#ifndef _MCTS_H_
#define _MCTS_H_

#define DEFAULT_N_SIMULATE 1

//Red for positive score, Black for negative score
typedef int Score;

class MCTS_agent{
    public:
        // MCTS_agent() = default;

        MCTS_agent(Color p_c, Position initial_pos, double initial_coeff = 1.0, int n_simulate_leaf = 5);
        void reset(Color p_c, Position initial_pos);
        void MCTS_simulatie(int N_simulate, double time_constraint = 5.0);
        bool MCTS_iteration();//return true if early-stop
        Move opt_solution(double time_constraint, int N_simulate = 0);
        
        Color player_color;
        // when expand, simulate n_simulate_expand times for each new leaves 
        int n_simulate_expand;


    
    //private:
        //node operations
            Position root_pos;
            Node *Nodes;
            int maximum_node_idx;

            int create_root(const Color &cur_color){
                Node root =  Node(cur_color,  maximum_node_idx);
                Nodes[maximum_node_idx] = root;
                return maximum_node_idx++;
            }

            int create_sucessor(int parent_idx){
                Nodes[maximum_node_idx] = Node(parent_idx, Nodes[parent_idx]);
                return maximum_node_idx++;
            }


        //MCTS ELEMENTS
            int root_idx;
            double Exploration_coeff;
            //return the index and the position of the leaf in PV
            pair<int, Position> search_pv();

            //find the maximum children of the given node
            int select_maximum_child_idx(int node_idx);
            // Node select_maximum_child_idx(int node_idx);

            void expand(int node_idx, Position node_pos);

            //return the result of simulate in a given number of simulation
            //w.r.t. the player of pos
            Score simulate(Position pos, int n_simulate);

            //used for back_propregation
            void update_node(int node_idx, Score score, int n_simulate);//w: number of winning, n: total number of simulation
            void back_propregation(int leaf_idx, Score score, int n_simulate);

        //utils for expansion
            //UCB-score of node_id w.r.t. its parent color
            long double UCB(int node_idx, int parent_idx);


};

#endif