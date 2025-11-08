// Wakasagihime
// Plays Chinese Dark Chess (Banqi)!

#include "lib/chess.h"
#include "lib/marisa.h"
#include "lib/types.h"
#include "lib/helper.h"
// #define TEST 1
// #define DEBUG 1
#define USE_MCTS 1

#ifdef USE_MCTS
#include"mcts_agent.h"
#endif

// Girls are preparing...
__attribute__((constructor)) void prepare()
{
    // Prepare the distance table
    for (Square i = SQ_A1; i < SQUARE_NB; i += 1) {
        for (Square j = SQ_A1; j < SQUARE_NB; j += 1) {
            SquareDistance[i][j] = distance<Rank>(i, j) + distance<File>(i, j);
        }
    }

    // Prepare the attack table (regular)
    Direction dirs[4] = { NORTH, SOUTH, EAST, WEST };
    for (Square sq = SQ_A1; is_okay(sq); sq += 1) {
        Board a = 0;
        for (Direction d : dirs){
            a |= safe_destination(sq, d);
        }
        PseudoAttacks[sq] = a;
    }

    // Prepare magic
    init_magic<Cannon>(cannonTable, cannonMagics);
}

// le fishe
int main()
{
    /*
     * This is a simple Monte Carlo agent, it does
     *     - move generation
     *     - simulation
     *
     * To make it good MCTS, you still need:
     *     - a tree
     *     - Some UCB math
     *     - other enhancements
     *
     * You SHOULD create new files instead of cramming everything in this one,
     * it MAY affect your readability score.
     */
    std::string line;
    /* read input board state */
    
    #ifdef USE_MCTS
        Position pos_init;
        MCTS_agent agent(Red, pos_init, 1, 5);
    #endif

    while (std::getline(std::cin, line)) {
        Position pos(line);
        
        #ifdef USE_MCTS
        
            agent.reset(pos.due_up(), pos);

            #ifdef TEST
                info << "current pos:------------------------\n";
                info << pos <<endl;
            #endif

            #ifdef DEBUG
                agent.MCTS_iteration();
                Node root = agent.Nodes[agent.root_idx];
                cout <<"N:" << root.Ntotal <<endl;
                cout<<"Child: "<<root.Nchild <<endl;
                for(int i=0; i<root.Nchild; i++){
                    cout<<"\tchild id:"<< root.c_id[i] <<"\n";
                    cout<<"\tmove:"<<root.c_move[i]<<endl;
                }
                cout<<"W:" << root.Mean <<endl;
                for(int i=0;i<agent.maximum_node_idx; i++){
                    Node node = agent.Nodes[i];
                    cout <<"node "<<i<<endl;
                    cout <<"\tN:" << node.Ntotal <<endl;
                    cout<<"\tChild: "<<node.Nchild <<endl;
                    cout<<"\tW:" <<node.score_sum <<"/" <<node.Ntotal <<"=" << node.Mean <<endl;

                }
                Move choice = agent.opt_solution(0,0);
                cout<<"winrate:"<<agent.Nodes[agent.root_idx].Mean <<endl;
                info <<"choice:" << choice;

                return 0;

            #endif

            agent.MCTS_simulatie(100);
            Move nx_move = agent.opt_solution(0,0);

            #ifdef DEBUG
            
                cout<<"winrate:"<<agent.Nodes[agent.root_idx].Mean <<endl;
                info <<"chlice:" << nx_move;

                return 0;
            #endif
        #else
            // -------OLD MCS-----------------------------

            MoveList moves(pos);

            #ifdef DEBUG
                info << pos;
            #endif

            int min_score = 100;
            int chosen = 0;

            for (int i = 0; i < moves.size(); i += 1) {
                Position copy(pos);
                copy.do_move(moves[i]);
                int local_score = 0;
                for (int j = 0; j < 20; j += 1) {
                    /* Run some (20) simulations. */
                    local_score += copy.simulate(strategy_random);
                }
                /*
                * The simulations started from the opponent's perspective,
                * so we choose the move that led to the MINIMUM score here.
                */
                if (local_score < min_score) {
                    chosen = i;
                    min_score = local_score;
                }
            }
            /* output the move */
            Move nx_move = moves[chosen];
        #endif

        info << nx_move;

        #ifdef TEST
            pos.do_move(nx_move);
            info << pos;
            info << pos.toFEN() <<endl;
            if(pos.winner() != NO_COLOR){
                info << "game end\n";
                info << ((pos.winner()==Black)?"BLACK":"WHITE") << '\n';
                // break;
            }
        #endif

    }
}
