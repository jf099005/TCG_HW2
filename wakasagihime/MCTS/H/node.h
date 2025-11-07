#ifndef _NODE_H_
#define _NODE_H_

#include "lib/chess.h"
#include "lib/marisa.h"
#include "lib/types.h"
#include "lib/helper.h"
const int MaxChild = 100;
const int MaxNode = 1e6;

using namespace std;

typedef struct MCT_node{
    Color color;
    int ply; // the ply from parent to here
    int p_id; // parent id, root’s parent is the root
    int c_id[MaxChild]; // children id
    int depth; // depth, 0 for the root
    int Nchild; // number of children
    int Ntotal; // total # of simulations
    long double CsqrtlogN; // c * sqrt(log(Ntotal))
    long double sqrtN; // sqrt(Ntotal)
    int sum1; // sum1: sum of scores
    int sum2; // sum2: sum of square of each score
    long double Mean; // average score
    long double Variance; // variance of score
    bool can_expand;

    //for root
    MCT_node(){
        color = Red;
        p_id = 0;
        depth = 0;
        Nchild = 0;
        Ntotal = 0;
        sum1 = 0;
        sum2 = 0;
        can_expand = 1;
    }
    MCT_node(Color cur_color, int root_id){
        color = cur_color;
        p_id = root_id;
        depth = 0;
        Nchild = 0;
        Ntotal = 0;
        sum1 = 0;
        sum2 = 0;
        can_expand = 1;
    }

    //for others
    MCT_node(Color cur_color, int parent_id, Node parent){
        color = cur_color;
        p_id = parent_id;
        depth = parent.depth + 1;
        Nchild = 0;
        Ntotal = 0;
        sum1 = 0;
        sum2 = 0;
        can_expand = 1;
    }
}
Node;

#endif