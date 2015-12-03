/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 *
 * Optimizations done in the game_of_life function call include:
 * 1) Parallelization of nested for loops using Open MP (8 threads)
 * 2) Reusing five out of the eight neighbour values with each iteration of 'j'
 * 3) LCIM - Moved computation of inorth and isouth outside the inner for loop
 * 4) Swap 'i' and 'j' in BOARD macro 
 ****************************************************************************/
#include "life.h"
#include "util.h"

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/

/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__j) + LDA*(__i)])
#define NUM_THREADS (8)

/**
 * game_of_life
 * This is the main function call that invokes the main game of life algorithm
 * This version is parallelized and optimized.
 * @param outbound - the output board array -> nrows x ncols
 * @param inbound - the input board array -> nrows x ncols
 * @param nrows - Total number of rows in the board
 * @param ncols - Total number of columns in the board
 * @param gens_max - Total number of ticks to evolve the input board 
 * @return char * - Returns the input board
 **/

char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
    /* HINT: in the parallel decomposition, LDA may not be equal to
    nrows! */
    const int LDA = nrows;
    int curgen;

    for (curgen = 0; curgen < gens_max; curgen++)
    {
        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
        /**
         * Pragma directive invoking Open MP parallelization for the two nester for loops
         * Ensured that i and j declarations happen within the scope of the open MP
         * parallelization so that each thread have their own dedicated i and j variables
         * **/
        #pragma omp parallel num_threads(NUM_THREADS)
        {
	        int i, j;   //Need these inside omp pragme so that they are not shared between threads
	        int thread_num = omp_get_thread_num();  //Gets the current threads num identifier

            //Split the outer for loop equally between all threads in NUM_THREADS
	        for (i = thread_num*nrows/NUM_THREADS ; i < (thread_num+1)*nrows/NUM_THREADS; i++)
            {
	        	const int inorth = mod (i-1, nrows);    //LCIM - mod only uses 'i' value
	        	const int isouth = mod (i+1, nrows);

                //Declare all eight neighbours and current cell.
                //Compute, north, north-east, current, east, south, and south-west
	        	char nw;
	        	char n  = BOARD (inboard, inorth, mod (-1, ncols));
	        	char ne = BOARD (inboard, inorth, 0);
	        	char w;
	        	char c  = BOARD (inboard, i, mod (-1, ncols));
	        	char e  = BOARD (inboard, i, 0);
	        	char sw;
	        	char s  = BOARD (inboard, isouth, mod (-1, ncols));
	        	char se = BOARD (inboard, isouth, 0);

                for (j = 0; j < ncols; j++)
                {

                    const int jwest = mod (j-1, ncols);
                    const int jeast = mod (j+1, ncols);

                    //Shift the neighbour values to the left
                    //This enables us to save computation in each stride of j
                    //Only need to compute three new values each stride:
                    //  north-east
                    //  east
                    //  south-east
                    nw = n;
                    n  = ne;
                    ne = BOARD (inboard, inorth, jeast);
                    w  = c;
                    c  = e;
                    e  = BOARD (inboard, i, jeast);
					sw = s;
					s  = se;
					se = BOARD (inboard, isouth, jeast);

                    const char neighbor_count = nw + n + ne + w + e + sw + s + se;

                    BOARD(outboard, i, j) = alivep (neighbor_count, c);

                }
            }
	    }
        SWAP_BOARDS( outboard, inboard );

    }
    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
    return inboard;
}
