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

#include <chrono>
using namespace std::chrono;


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
    int remain_moves = 32;
    int num_pieces = 32;
    MCTS_agent agent(Red, pos_init, 50.0, 10);

    while (getline(std::cin, line)) {
        Position pos(line);
        AlphaBetaEndgameSolver ab_solver(pos.due_up(), ab_depth);
    
        auto start = high_resolution_clock::now();


        debug << pos;
        debug << "remain moves:" << remain_moves <<endl;

        // Color endgame = is_endgame(pos);
        int red_count = pos.count(Red);
        int black_count = pos.count(Black);
        if( red_count + black_count != num_pieces){
            remain_moves = 32;
            num_pieces = red_count + black_count;
        }
        else{
            remain_moves--;
        }

        // if(num_black_pieces > black_count){
        //     num_black_pieces = black_count;
        //     remain_moves = 40;
        // }

        // if(endgame != NO_COLOR and endgame == pos.due_up()){
        // remain_moves = 27;
        int opponent_count = (pos.due_up() != Red?red_count:black_count);
        opponent_count = 100;
        // remain_moves = 1;

            debug << "mcts mode\n";
            agent.reset(pos.due_up(), pos, remain_moves);
            
            agent.MCTS_simulate(5, 0.01);
            debug << "initial beta: "<< agent.calculate_beta(agent.root) <<endl;

            agent.MCTS_simulate(N_simulate, 1.0);

            debug << "terminal beta: "<< agent.calculate_beta(agent.root) <<endl;
            agent.print_node(agent.root);

            Move nx_move = agent.opt_solution();
            // debug << "mv:" << nx_move;
            // debug << "N:" << agent.root->Ntotal << ", AMAF:" << \
            //         agent.get_AMAF_Node(agent.root)->Ntotal <<endl;

            debug <<"total cut:" << agent.total_cut << ", total reconnection:" << agent.total_reconnect <<endl;
            debug << "winrate:" << agent.root->Mean <<", AMAF winrate:"<<\
                    agent.get_AMAF_Node(agent.root)->Mean <<endl;
            debug << nx_move<<endl;

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(stop - start);
            debug << "time:" << double(duration.count())*microseconds::period::num/microseconds::period::den << endl;
            debug << "node usage:" << agent.maximum_node_idx <<endl;
            pos.do_move(nx_move);


            info << nx_move;

        remain_moves--;

        if(pos.count(ALL_PIECES) != num_pieces){
            remain_moves = 32;
        }

        // return 0;
    }
}
