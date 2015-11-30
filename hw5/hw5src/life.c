/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
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
        #pragma omp parallel num_threads(NUM_THREADS)
        {
	        int i, j;
	        int thread_num = omp_get_thread_num();

	        for (i = thread_num*nrows/NUM_THREADS ; i < (thread_num+1)*nrows/NUM_THREADS; i++)
            {
	        	const int inorth = mod (i-1, nrows);
	        	const int isouth = mod (i+1, nrows);

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
