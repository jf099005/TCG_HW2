// Wakasagihime
// Plays Chinese Dark Chess (Banqi)!

#include "lib/chess.h"
#include "lib/marisa.h"
#include "lib/types.h"
#include "lib/helper.h"
// #define TEST 1
#define DEBUG 1
#define USE_MCTS 1
#ifdef USE_MCTS
#include"mcts_agent.h"
#endif

#define MY_AGENT 1

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
using namespace std;
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
        #ifndef ONLY_MCS
            int N_simulate = 1000;
            MCTS_agent agent(Red, pos_init, 1.0, 1);
        #else
            int N_simulate = 1;
            MCTS_agent agent(Red, pos_init, 1, 1000);
        #endif
    #endif
    cout<<"initial N:" << agent.N <<"/" <<agent.N_AMAF <<endl;

    while (std::getline(std::cin, line)) {
        Position pos(line);
        
        #ifdef USE_MCTS
        
            agent.reset(pos.due_up(), pos);

            #ifdef TEST
                info << "current pos:------------------------\n";
                info << pos <<endl;
            #endif
            Move nx_move;
            for(int i=0; i<10; i++){
                agent.MCTS_simulate(100);
                nx_move = agent.opt_solution();

                cout<<"N:" << agent.N <<"/" <<agent.N_AMAF <<endl;

                cout<<"winrate:"<<agent.Nodes[agent.root_idx].Mean <<"/" <<\
                        agent.AMAF_Nodes[agent.root_idx].Mean <<endl;
                cout<<"amaf:" << agent.Nodes[agent.root_idx].score_sum <<'/' << agent.Nodes[agent.root_idx].Ntotal <<endl;
                info <<"chlice:" << nx_move;
            }
            
            return 0;
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

        #ifdef MY_AGENT
            pos.do_move(nx_move);
            info << pos <<endl;
            info << nx_move;
            info << pos.toFEN();
            return 0;
        #endif
    }
}
