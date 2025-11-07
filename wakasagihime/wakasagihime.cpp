// Wakasagihime
// Plays Chinese Dark Chess (Banqi)!

#include "lib/chess.h"
#include "lib/marisa.h"
#include "lib/types.h"
#include "lib/helper.h"
#define DEBUG 1
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
    while (std::getline(std::cin, line)) {
        Position pos(line);
        
        #ifdef USE_MCTS
            MCTS_agent agent(pos.due_up(), pos, 1, 5);
            #ifdef DEBUG
                info <<"root idx: " << agent.root_idx <<endl;
                info << "pv at beginning:" << agent.search_pv(pos) <<endl;
            #endif

            agent.MCTS_simulatie(100, 100);

            #ifdef DEBUG
                info << "candidate moves after search:\n";
                info <<agent.Nodes[agent.root_idx].Nchild <<endl;
                for(int i=0;i<agent.Nodes[agent.root_idx].Nchild; i++){
                    info << agent.Nodes[agent.root_idx].c_move[i] <<endl;
                }
            #endif
            Move nx_move = agent.opt_solution(100, 0);
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

        #ifdef DEBUG
            pos.do_move(nx_move);
            info << pos;
            if(pos.winner() != NO_COLOR){
                info << "game end\n";
                info << pos.winner() << '\n';
                break;
            }
        #endif

    }
}
