// Wakasagihime
// Plays Chinese Dark Chess (Banqi)!

#include "lib/chess.h"
#include "lib/marisa.h"
#include "lib/types.h"
#include "lib/helper.h"
#define USE_MCTS 1

// #define ONLY_MCS 1

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
        #ifdef ONLY_MCS
            int N_simulate = 1;
            MCTS_agent agent(Red, pos_init, 1, 1000);
        #else
            int N_simulate = 10000;
            MCTS_agent agent(Red, pos_init, 1.0, 5);
        #endif
    #endif

    while (std::getline(std::cin, line)) {
        Position pos(line);
        
        agent.reset(pos.due_up(), pos);
        agent.MCTS_simulate(N_simulate);

        Move nx_move = agent.opt_solution();

        info << nx_move;

        #ifdef MY_AGENT
            pos.do_move(nx_move);
            info << pos <<endl;
            info << nx_move;
            info << pos.toFEN();
            return 0;
        #endif
    }
}
