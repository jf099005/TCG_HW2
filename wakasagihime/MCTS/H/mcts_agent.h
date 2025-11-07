#include"node.h"
#ifndef _MCTS_H_
#define _MCTS_H_

#define DEFAULT_N_SIMULATE 1

class MCTS_agent{
    //public:
        MCTS_agent(Color p_c);

        Move opt_solution(Position pos, int N_simulate);
        const Color player_color;

    
    //private:

        //node generation
            Node Nodes[MaxNode];
            int current_node_idx;
            inline int gen_new_node(){
                return current_node_idx++;
            };    

        //MCTS ELEMENTS
            int root_idx;

            //return the index of the leaf in PV
            int search_pv();

            //find the maximum children of the given node
            int select_maximum(int node_idx);

            void expand(int node_id);

            //return the result of simulate in a given number of simulation
            int simulate(Position pos, int n_simulate = DEFAULT_N_SIMULATE);

            void update_node(int node_id, int w, int n);//w: number of winning, n: total number of simulation
            void back_propregation(int leaf_idx, int result);

        //utils
            long double UCB(int node_id);



};

#endif