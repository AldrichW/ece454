/*
 * Things in my control:
 *     - The size of internal fragmentation
 *     - The size of memory increase (sbrk call)
 *     - The number of segregated free lists and their values
 *     - Last In First Out and First Fit - for free list
 *     - If no free block of appropriate size is found, check if
 *       the last block in the heap is free, and coalesce with newly
 *       requested memory to increase memory utilization
 *
 */

/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "team_almost_there",
    /* First member's full name */
    "Suhaib Ahmed",
    /* First member's email address */
    "suhaib.ahmed@mail.utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "Aldrich Wingsiong",
    /* Second member's email address (leave blank if none) */
    "aldrich.wingsiong@mail.utoronto.ca"
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Get the next and prev pointer to free block given pointer to header of a free block */
#define GET_NEXT_PTR(p)  (GET((char*)(p)+WSIZE))
#define GET_PREV_PTR(p)  (GET((char*)(p)+DSIZE))

/* Set the next and prev pointer to free block given pointer to header of a free block */
#define SET_NEXT_PTR(p,val)  (PUT((char*)(p)+WSIZE, val))
#define SET_PREV_PTR(p,val)  (PUT((char*)(p)+DSIZE, val))

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define HASH_SIZE 32

void* heap_listp = NULL;
static void * segList[HASH_SIZE];

/**********************************************************
 * Hashing function that just calculates the log of 'key'
 *
 * @param key = integer who's hash needs to be computed
 *
 * @return value of the hash index
 *
 **********************************************************/
int log_hash(int key)
{
    int val = 0;
    while (key >>= 1)
    {
        val+=1;
    }

    return val;
}

void add_to_seglist(void * free_block)
{
	assert (free_block != NULL);
	assert (!GET_ALLOC(free_block));

	int index = log_hash(GET_SIZE(free_block));
	void* old_first_block = segList[index];
	segList[index] = free_block;

	SET_NEXT_PTR(free_block, (uintptr_t)old_first_block);
	SET_PREV_PTR(free_block, (uintptr_t)NULL);

	if (old_first_block)
	{
		SET_PREV_PTR(old_first_block, (uintptr_t)free_block);
	}

	return;
}

int is_block_in_free_list(void * block)
{
	assert (block != NULL);

	int index = log_hash(GET_SIZE(block));
	int true = 0;
	void * curr_ptr = segList[index];

	while (curr_ptr!=NULL)
	{
		if (block == curr_ptr)
		{
			true = 1;
		}
		curr_ptr = (void*)GET_NEXT_PTR(curr_ptr);
	}

	return true;
}

void remove_from_seglist(void * free_block)
{
	assert (free_block != NULL);
	assert (!GET_ALLOC(free_block));
	assert (is_block_in_free_list(free_block) == 1);

	uintptr_t next = GET_NEXT_PTR(free_block); // next pointer
	uintptr_t prev = GET_PREV_PTR(free_block); // prev pointer

	SET_PREV_PTR(next, prev); // next's prev = prev
	SET_NEXT_PTR(prev, next); // prev's next = next

	return;
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        {return -1;}
    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += DSIZE;

    int itr=0;
    for(; itr<HASH_SIZE; itr++)
    {
    	segList[itr] = (void *)NULL;
    }

    return 0;
}

//int mm_init(void)
//{
//    if ((heap_listp = mem_sbrk(20*WSIZE)) == (void *)-1)
//        {return -1;}
//    PUT(heap_listp, PACK(4 * WSIZE, 0));
//    PUT(heap_listp + (1 * WSIZE), 0xaa00aa);
//    PUT(heap_listp + (2 * WSIZE), 0x00aa00);
//    add_to_seglist(heap_listp);
//
//    PUT(heap_listp + (4 * WSIZE), PACK(4 * WSIZE, 0));
//    PUT(heap_listp + (5 * WSIZE), 0xbb00bb);
//    PUT(heap_listp + (6 * WSIZE), 0x00bb00);
//    add_to_seglist(heap_listp + (4 * WSIZE));
//
//    PUT(heap_listp + (8 * WSIZE), PACK(4 * WSIZE, 0));
//    PUT(heap_listp + (9 * WSIZE), 0xcc00cc);
//    PUT(heap_listp + (10 * WSIZE), 0x00cc00);
//    add_to_seglist(heap_listp + (8 * WSIZE));
//
//    PUT(heap_listp + (12 * WSIZE), PACK(4 * WSIZE, 0));
//    PUT(heap_listp + (13 * WSIZE), 0xdd00dd);
//    PUT(heap_listp + (14 * WSIZE), 0x00dd00);
//    add_to_seglist(heap_listp + (12 * WSIZE));
//
//    PUT(heap_listp + (16 * WSIZE), PACK(4 * WSIZE, 0));
//    PUT(heap_listp + (17 * WSIZE), 0xee00ee);
//    PUT(heap_listp + (18 * WSIZE), 0x00ee00);
//    add_to_seglist(heap_listp + (16 * WSIZE));
//
//    remove_from_seglist(heap_listp + (8 * WSIZE));
//    remove_from_seglist(heap_listp + (8 * WSIZE));
//
//    return 0;
//}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp)
{
    /******************************************************
     * Steps for coalescing:
     * 1) Check if two blocks beside current block is free
     * 2) If there is a block to the left, update 
     *    its header with new length
     *       a)Otherwise, update current block's header 
     *         with new length
     * 3) If there is a block to the right, update its 
     *    footer with new length
     *       b)Otherwise, update current block's footer 
     *         with new length
     *
     * NOTE: I'm assuming that the current block bp was
     * never in the free list to begin with. May be worth
     * revisiting.
     ******************************************************/
    assert(!GET_ALLOC(HDRP(bp)));

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    //Let's calculate the header pointer of all three blocks
    void *prev_header = HDRP(PREV_BLKP(bp));
    void *curr_header = HDRP(bp);
    void *next_header = HDRP(NEXT_BLKP(bp));

    //Let's calculate the footer pointer of all three blocks
    // void *prev_footer = FTRP(PREV_BLKP(bp));
    void *curr_footer = FTRP(bp);
    void *next_footer = FTRP(NEXT_BLKP(bp));

    
    if (prev_alloc && next_alloc) {       /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
        // Remove next block from the appropriate free list
        remove_from_seglist(next_header);

        // Merge the prev and curr blocks
        size += GET_SIZE(next_header);

        PUT(curr_header, PACK(size, 0));
        PUT(next_footer, PACK(size, 0));
        // Add the new block to the approproate free list
        
        add_to_seglist(curr_header);
        
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */

        // Remove next block from the appropriate free list
        remove_from_seglist(prev_header);
        
        // Merge the next and curr blocks
        size += GET_SIZE(prev_header);
        PUT(curr_footer, PACK(size, 0));
        PUT(prev_header, PACK(size, 0));

        // Add the new block to the approproate free list
        add_to_seglist(prev_header);

        return (PREV_BLKP(bp));
    }

    else {            /* Case 4 */
        // Remove the prev and next block from the appropriate free lists
        remove_from_seglist(prev_header);
        remove_from_seglist(next_header);

        // Merge the prev, curr, and next blocks
        size += GET_SIZE(prev_header)  +
            GET_SIZE(next_footer)  ;
        PUT(prev_header, PACK(size,0));
        PUT(next_footer, PACK(size,0));

        // Add the new block to the appropriate free list
        add_to_seglist(prev_header);

        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ( (bp = mem_sbrk(size)) == (void *)-1 )
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}


/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            return bp;
        }
    }
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
  /* Get the current block size */
  size_t bsize = GET_SIZE(HDRP(bp));

  PUT(HDRP(bp), PACK(bsize, 1));
  PUT(FTRP(bp), PACK(bsize, 1));
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{
    if(bp == NULL){
      return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    coalesce(bp);
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{
    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0){
      mm_free(ptr);
      return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
      return (mm_malloc(size));

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void){
    return 1;
}
