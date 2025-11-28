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
#include"AlphaBetaEndgame/H/AB_agent.h"
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
    
    while (getline(std::cin, line)) {
        Position pos(line);
        AlphaBetaEndgameSolver ab_solver(pos.due_up());
        info << pos;

        ab_solver.Negamax(pos, 4);

        info << ab_solver.opt_solution;
    }
}
