#ifndef _util_h
#define _util_h

/**
 * C's mod ('%') operator is mathematically correct, but it may return
 * a negative remainder even when both inputs are nonnegative.  This
 * function always returns a nonnegative remainder (x mod m), as long
 * as x and m are both positive.  This is helpful for computing
 * toroidal boundary conditions.
 */
static inline int 
mod (int x, int m)
{
    if( x < 0 ){
        return (x%m) +m;
    }

    if( x >= m )
    {
	    return x%m;
    }

    return x;
}

/**
 * Given neighbor count and current state, return zero if cell will be
 * dead, or nonzero if cell will be alive, in the next round.
 */
static inline char 
alivep (char count, char state)
{
    // return (! state && (count == (char) 3)) || (state && (count >= 2) && (count <= 3));
    return ((count == (char) 3) || (state && (count == (char) 2)));
}

#endif /* _util_h */
