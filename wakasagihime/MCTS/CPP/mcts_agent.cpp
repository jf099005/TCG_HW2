#ifndef _MCTS_CPP_
#define _MCTS_CPP_

#include"mcts_agent.h"

MCTS_agent::MCTS_agent(Color p_c):
        player_color(p_c)
    {
        this->current_node_idx = 0;
        this->root_idx = 0;
        
    };



int MCTS_agent::search_pv(){
    int cur_node_idx = root_idx;
    while(!Nodes[cur_node_idx].can_expand){
        int max_nx = this->select_maximum(cur_node_idx);
        cur_node_idx = max_nx;
    }
    return cur_node_idx;
}


#endif