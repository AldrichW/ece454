/* mm.c
 * This allocator implements a segregated free list to
 * manage all the free blocks based on size classes. 
 * Each index in the hash table represents the size of
 * the block as a power of 2 with a max size of 64. 
 * 
 * Allocated blocks contain an 8 byte header and a 
 * minimum of 24 bytes for the payload. The lowest
 * bit of the header indicates whether the block 
 * is allocated (1) or free (0). In this case, 
 * the bit is set to 1. The rest of the 61 bits 
 * are used for the size since each block is 
 * a multiple of 8.
 *
 * Freed blocks contain an 8 byte header and 8 byte 
 * footer which contain the size and the allocated bit
 * as the lowest bit. In this case it will be set to 0.
 * In addition, freed blocks will store the pointer to the
 * next block's header within the word following its own
 * header. A pointer to the previous block's header is stored
 * in the next word after.
 *
 * When a block is freed, the allocator bit is set to 0 and
 * goes through coalesce process. The block is then 
 * added to the segregated free list. The block is hashed 
 * using its block size and added to the front of
 * the linked list of blocks.
 *
 * When malloc and realloc are called, the hash table is accessed
 * based on the size argument passed in (minimum being 32 bytes).
 * From there the first block is taken from the linked list.
 *
 * Blocks are coalesced when a block is free'd or when
 * an edge case is encountered where the last block of the
 * heap is free and the heap needs to be extended. In this 
 * case, the new block will be composed of the existing free
 * block and a new heap extension with size being the difference
 * between the size argument and the free block available.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 *
 * Function Declarations
 *
 ********************************************************/
void * coalesce(void *bp);                      //coalesces the block pointed at by bp. Checks all four cases.
void * extend_heap(size_t size);                //Extends the heap utilizing the free blocks in the segregated list. Keeps current contents.
void * get_fit(size_t asize);                   //Defines the policy for finding a free block that fits the size argument.
void   place(void* bp, size_t asize);           //Marks the header and footer of the block as allocated with the size argument. 

size_t    log_hash(size_t key);                 //Hashes the key and converts it to an index for the segregated free list hash table
void   add_to_seglist(void * free_block);       //Adds the free block to the segregated list
void   remove_from_seglist(void * free_block);  //Removes a free block from the segregated list
bool   is_block_in_seglist(void * block);       //Quick check to see if a block is in the segregated list.
bool   is_block_in_freelist(void * block);      //Quick check to see if a block is in the free list.


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

#define HASH_SIZE 64

void* epilogue_ptr = NULL;
static void * segList[HASH_SIZE];
bool dont_coalesce = false;

/**********************************************************
 * Hashing function that just calculates the log of 'key'
 *
 * @param key = integer who's hash needs to be computed
 *
 * @return value of the hash index
 *
 **********************************************************/
size_t log_hash(size_t key)
{
    size_t index=0;
    int val = 1;
    for (index = 0 ; index < HASH_SIZE ; index++)
    {
        if (key <= val)
        {
        	break;
        }
        val<<=1;
    }
    return index;
}

/**********************************************************
 * is_block_in_seglist
 * Checks to see if a block exists in the segregated list.
 *
 * @param block - the block pointer in question
 *
 * @return bool - true if the block is in the list. 
 *                False otherwise.
 *
 **********************************************************/

bool is_block_in_seglist(void * block)
{
	assert (block != NULL);

	size_t index = 0;

	while (index < HASH_SIZE)
	{
		void * list_root = segList[index];
		while (list_root!=NULL)
		{
			if (block == list_root)
			{
				return true;
			}
			list_root = (void *)GET_NEXT_PTR(list_root);
		}
		index ++;
	}

	return false;
}

/**********************************************************
 * print_segList
 * Prints the entire segregated list in a human readable
 * form using standard out.
 *
 **********************************************************/
void print_segList(){
    int i;
    
    for(i = 0; i < HASH_SIZE; i++){
        printf("Hash index:%d\n================\n", i);
        if(segList[i] != NULL){
            //Traverse through linked list
            int count = 0;
            void *currHeader = segList[i];
            while(currHeader){
                printf("Block: %d\n", count);
                printf("Size: %d\n\n",GET_SIZE(currHeader));
                currHeader = (void*)GET_NEXT_PTR(currHeader);
                count++;
            }

            printf("\n");
        }
        else{
            printf("Index is empty!\n\n\n");
        }
    }
}

/**********************************************************
 * add_to_seglist
 * Adds a block that has been set as free to the segregated 
 * free list.
 *
 * @param free_block - a pointer to the free block being
 *                     added.
 *
 * @return void
 *
 **********************************************************/
void add_to_seglist(void * free_block)
{
	size_t index = log_hash(GET_SIZE(free_block));
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

/**********************************************************
 * is_block_in_freelist
 * Checks to see if a block exists in the free list.
 *
 * @param block - the block pointer in question
 *
 * @return bool - true if the block is in the list. 
 *                false otherwise.
 *
 **********************************************************/

bool is_block_in_freelist(void * block)
{
//	printf("Calling %s \n", __FUNCTION__);

	assert (block != NULL);

	size_t index = log_hash(GET_SIZE(block));
	void * list_root = segList[index];

	while (list_root!=NULL)
	{
		if (block == list_root)
		{
			return true;
		}
		list_root = (void *)GET_NEXT_PTR(list_root);
	}

	return false;
}
/**********************************************************
 * remove_from_seglist
 * Removes a free block from the segregated list
 *
 * @param free_block - a pointer to the free block being
 *                     being removed.
 *
 * @return void
 *
 **********************************************************/

void remove_from_seglist(void * free_block)
{
	int index = log_hash(GET_SIZE(free_block));
	
    uintptr_t next = GET_NEXT_PTR(free_block); // next pointer
	uintptr_t prev = GET_PREV_PTR(free_block); // prev pointer

	if (next)
	{
		SET_PREV_PTR(next, prev); // next's prev = prev
	}

	if (prev)
	{
		SET_NEXT_PTR(prev, next); // prev's next = next
	}
	else
	{
		int index = log_hash(GET_SIZE(free_block));
		segList[index] = (void *)next;
	}

	return;
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
    void* heap_listp;
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        {return -1;}
    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    epilogue_ptr = heap_listp + (3 * WSIZE);

    heap_listp += DSIZE;

    int itr=0;
    for(; itr<HASH_SIZE; itr++)
    {
    	segList[itr] = (void *)NULL; //initialize each element in the segregated free list to NULL
    }

    return 0;
}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 *
 *   @param bp - the block pointer to be coalesced
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
    
    //store the flags for whether or not the the previous block is allocated
    //and whether or not the next flag is allocated.
    //This is mainly used as conditional logic for the four cases
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    //Let's calculate the header pointer of all three blocks
    void *prev_header = HDRP(PREV_BLKP(bp));
    void *curr_header = HDRP(bp);
    void *next_header = HDRP(NEXT_BLKP(bp));
    printf("NextHeader: %p\n",next_header);
    printf("PrevHeader: %p\n",prev_header);

    //Let's calculate the footer pointer of all three blocks
    // void *prev_footer = FTRP(PREV_BLKP(bp));
    void *curr_footer = FTRP(bp);
    void *next_footer = FTRP(NEXT_BLKP(bp));
    
    if (prev_alloc && next_alloc)              /* Case 1 - No coalescing necessary*/
    {
    	return bp;
    }

    else if (prev_alloc && !next_alloc)        /* Case 2 - Coalesce current block with the next block */
    {
        // Remove next block from the appropriate free list
        remove_from_seglist(next_header);

        // Merge the prev and curr blocks
        size += GET_SIZE(next_header);

        PUT(curr_header, PACK(size, 0));
        PUT(next_footer, PACK(size, 0));
        
        return (bp);
    }

    else if (!prev_alloc && next_alloc)       /* Case 3 - Coalesce current block with the previous  */
    {
        // Remove next block from the appropriate free list
        remove_from_seglist(prev_header);
        
        // Merge the next and curr blocks
        size += GET_SIZE(prev_header);
        PUT(curr_footer, PACK(size, 0));
        PUT(prev_header, PACK(size, 0));

        return (PREV_BLKP(bp));
    }

    else            /* Case 4 - Coalesce current block with both the previous and next block*/
    {
        // Remove the prev and next block from the appropriate free lists
        remove_from_seglist(prev_header);
        remove_from_seglist(next_header);

        // Merge the prev, curr, and next blocks
        size += GET_SIZE(prev_header)  +
            GET_SIZE(next_footer)  ;
        PUT(prev_header, PACK(size,0));
        PUT(next_footer, PACK(size,0));

        return (PREV_BLKP(bp));
    }
}
/*
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(20*WSIZE)) == (void *)-1)
        {return -1;}
    PUT(heap_listp, PACK(4 * WSIZE, 0));
    PUT(heap_listp + (1 * WSIZE), 0xaa00aa);
    add_to_seglist(heap_listp);

    print_segList();

    PUT(heap_listp + (4 * WSIZE), PACK(4 * WSIZE, 0));
    PUT(heap_listp + (5 * WSIZE), 0xbb00bb);
    add_to_seglist(heap_listp + (4 * WSIZE));

    print_segList();

    PUT(heap_listp + (8 * WSIZE), PACK(4 * WSIZE, 0));
    PUT(heap_listp + (9 * WSIZE), 0xcc00cc);
    add_to_seglist(heap_listp + (8 * WSIZE));

    print_segList();

    PUT(heap_listp + (12 * WSIZE), PACK(4 * WSIZE, 0));
    PUT(heap_listp + (13 * WSIZE), 0xdd00dd);
    add_to_seglist(heap_listp + (12 * WSIZE));

    print_segList();

    PUT(heap_listp + (16 * WSIZE), PACK(4 * WSIZE, 0));
    PUT(heap_listp + (17 * WSIZE), 0xdd00ee);
    add_to_seglist(heap_listp + (16 * WSIZE));
    
    print_segList();
   
    remove_from_seglist(heap_listp);

    print_segList();
    coalesce(heap_listp);

    return 0;
}
*/
/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t size)
{
    char *bp;

    // If the previous block is free, only extend the heap by (size - size_of_prev_free_block) so that you reduce external fragmentation
    size_t size_prev_free = GET_SIZE(epilogue_ptr-WSIZE) * !GET_ALLOC(epilogue_ptr-WSIZE);
    size -= size_prev_free;

    assert (size % DSIZE == 0);

    if ( (bp = mem_sbrk(size)) == (void *)-1 )
    {
        return NULL;
    }

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    epilogue_ptr = HDRP(NEXT_BLKP(bp));

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/**********************************************************
 * break_block_and_return_bp
 * Breaks the block into two segments: one block consisting 
 * of asize bytes and another with a size
 * of original block size minus asize
 *
 * @param block - A pointer to the block
 * @param asize - The size needed from the block
 *
 * @return void * the pointer to the block with size asize
 *
 **********************************************************/

void * break_block_and_return_bp(void * block, size_t asize)
{
	size_t block_size = GET_SIZE(block);
	remove_from_seglist(block);

	size_t fragment_size = block_size - asize;

	// If fragment is the minimum size of a free block (4 words), break it up
	if (fragment_size >= WSIZE*4)
	{
		// Create a block of asize
		PUT(block, PACK(asize,0));
		PUT(block+asize-WSIZE, PACK(asize,0));

		// Create a block of fragment_size
		PUT(block+asize, PACK(fragment_size,0));
		PUT(block+block_size-WSIZE, PACK(fragment_size,0));

		// Add the new fragment to the segList
		add_to_seglist(block+asize);
	}

	return block+WSIZE;
}

/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * get_fit(size_t asize)
{
	int index = log_hash(asize);
	void * list_itr;

	while (index < HASH_SIZE)
	{
		list_itr = segList[index];
		while (list_itr!=NULL)
		{
            //First fit search.
            //Split the free block and allocate the necessary bytes
            //The half that remains free will be added to the correct 
            //size class in the segregated list.
			if (GET_SIZE(list_itr) >= asize)
			{
				return break_block_and_return_bp(list_itr, asize);
			}
			list_itr = (void *)GET_NEXT_PTR(list_itr);
		}
		index ++;
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
    if (!dont_coalesce) { bp = coalesce(bp); }
    add_to_seglist(HDRP(bp));
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
//	printf("Calling %s \n", __FUNCTION__);

    size_t asize; /* adjusted block size */
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
    if ((bp = get_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    if ((bp = extend_heap(asize)) == NULL)
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
//	printf("Calling %s \n", __FUNCTION__);

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0)
    {
      mm_free(ptr);
      return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
    {
      return (mm_malloc(size));
    }

    void *oldptr = ptr;
    void *newptr;
    size_t copySize = GET_SIZE(HDRP(oldptr));
    size_t asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

    /* If the size is big enough, return as is */
    if (copySize >= asize)
    {
    	return oldptr;
    }


    /* Free before a dynamic allocation to reduce fragmentation. If the free block 
     * is at the end, it will be reused with the part of the heap that we will extend. In
     * order to preserve the data in the free block, we need to prevent it from 
     * coalescing with the previous block so that it is not broken up later at any
     * arbitrary point and the data at those points overwritten by headers and footers
     *
     *  We also need to save the 2 words where next and previous pointers will be saved
     */
    uintptr_t word1 = GET(oldptr);
    uintptr_t word2 = GET(oldptr+WSIZE);
    
    dont_coalesce = true;
    mm_free(oldptr);
    dont_coalesce = false;

    newptr = mm_malloc(size*2);
    if (newptr == NULL)
    {
        return NULL;
    }

    /* Copy the old data. */
    if (size < copySize)
    {
        copySize = size;
    }
    memcpy(newptr, oldptr, copySize);

    /* Write back the 2 words that were overwritten by next and previous pointers */
    PUT(newptr, word1);
    PUT(newptr+WSIZE, word2);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void)
{
//	printf("Calling %s \n", __FUNCTION__);
    // Is every block in the free list marked as free?
        //Iterate through all indices and go through linked list. If a block is not set to free, output error log￼￼￼￼￼￼￼￼￼￼￼￼￼￼
    // Are there any contiguous free blocks that somehow escaped coalescing? 
    // 
    // Is every free block actually in the free list?
    // 
    // Do the pointers in the free list point to valid free blocks?
    // 
    // Do any allocated blocks overlap?
    // 
    // Do the pointers in a heap block point to valid heap addresses?
    return 1;
}
