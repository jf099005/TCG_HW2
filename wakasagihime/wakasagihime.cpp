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
#include"AB_agent.h"
#include"BoardAnalyze/H/board_analyzer.h"
#endif

// #define MY_AGENT 1

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
    Position pos_init;        

    int N_simulate = 1000;
    int ab_depth = 8;
    int remain_moves = 30;
    int num_red_pieces = 16;
    int num_black_pieces = 16;
    MCTS_agent agent(Red, pos_init, 1.0, 1);

    while (getline(std::cin, line)) {
        Position pos(line);
        AlphaBetaEndgameSolver ab_solver(pos.due_up(), ab_depth);
        
        debug << pos;

        // Color endgame = is_endgame(pos);
        int red_count = pos.count(Red);
        int black_count = pos.count(Black);
        if(num_red_pieces > red_count){
            num_red_pieces = red_count;
            remain_moves = 30;
        }

        if(num_black_pieces > black_count){
            num_black_pieces = black_count;
            remain_moves = 30;
        }

        // if(endgame != NO_COLOR and endgame == pos.due_up()){
        remain_moves = 27;
        int opponent_count = (pos.due_up() != Red?red_count:black_count);
        if(opponent_count <= 3){
            debug << "endgame mode\n";
            ab_solver.Negamax(pos, ab_depth, remain_moves);

            info << ab_solver.opt_solution;
        }
        else{

            debug << "mcts mode\n";
            agent.reset(pos.due_up(), pos);
            agent.MCTS_simulate(N_simulate);

            Move nx_move = agent.opt_solution();
            debug << "mv:" << nx_move;
            debug << "N:" << agent.N << ", AMAF:" << agent.N_AMAF <<endl;

            info << nx_move;
        }

        remain_moves--;
        // return 0;
    }
}
