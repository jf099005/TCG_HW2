#ifndef _NODE_H_
#define _NODE_H_

#include "lib/chess.h"
#include "lib/marisa.h"
#include "lib/types.h"
#include "lib/helper.h"
const int MaxChild = 100;
const int MaxNode = 1000000;

using namespace std;

typedef struct MCT_node{
    // Color color;//current player color
    
    int id;
    // int p_id; // parent id, root’s parent is the root
    MCT_node* parent;
    int  c_id[MaxChild]; // children id
    // Move c_move[MaxChild];
    Move move;
    int depth; // depth, 0 for the root
    int Nchild; // number of children
    int Ntotal; // total # of simulations
    long double CsqrtlogN; // c * sqrt(log(Ntotal))
    long double sqrtN; // sqrt(Ntotal)
    long double score_sum; // score_sum: sum of scores
    long double sq_score_sum; // sq_score_sum: sum of square of each score
    long double Mean; // average score
    long double Variance; // variance of score
    bool can_expand;

    //for root
    MCT_node(){
        // color = Red;
        parent = nullptr;
        // p_id = 0;
        depth = 0;
        Nchild = 0;
        Ntotal = 0;
        score_sum = 0;
        sq_score_sum = 0;
        can_expand = 1;
    }
    MCT_node(Color cur_color, int root_id){
        // color = cur_color;
        // p_id = root_id;
        parent = nullptr;
        id = root_id;
        depth = 0;
        Nchild = 0;
        Ntotal = 0;
        score_sum = 0;
        sq_score_sum = 0;
        can_expand = 1;
    }

    //for others
    // MCT_node(int parent_id, const MCT_node& parent, Move child_move){
    //     // color = parent.color;
    //     p_id = parent_id;
    //     depth = parent.depth + 1;
    //     Nchild = 0;
    //     Ntotal = 0;
    //     score_sum = 0;
    //     sq_score_sum = 0;
    //     can_expand = 1;
    //     move = child_move;

    // }


    MCT_node(int idx, MCT_node* parent, Move child_move){
        // color = parent.color;
        // p_id = parent_id;
        this->parent = parent;
        this->id = idx;
        depth = parent->depth + 1;
        Nchild = 0;
        Ntotal = 0;
        score_sum = 0;
        sq_score_sum = 0;
        can_expand = 1;
        move = child_move;
    }

    /*    constexpr Move &operator=(const Move &other)
    {
        raw = other.raw;
        return *this;
    }*/
    // constexpr MCT_node &operator = (const MCT_node& other) = default;
}
Node;

#endif