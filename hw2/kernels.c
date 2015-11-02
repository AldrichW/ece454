/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

#define BLOCK 32

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "WinningSqua",              /* Team name */

    "Aldrich Wingsiong",     /* First member full name */
    "aldrich.wingsiong@mail.utoronto.ca",  /* First member email address */

    "Suhaib Ahmed",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
        dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 
char rotate_attempt2_descr[] = "rotate attempt 2: Let's try reducing the traversal through the for loops";
void rotate_attempt2(int dim, pixel *src, pixel *dst) {
    int i, j;
    int dimming = dim*dim-dim;
    for (i = 0; i < dim; i++){
      int dst_row = dimming+i;
      int src_row = i*dim;
      for(j = 0; j < dim; j++){
	int dst_index = dst_row - (j*dim);
	int src_index = src_row + j;
	dst[dst_index] = src[src_index];
      } 
    }

    //Lame, the compiler optimizes for this already.
}

char rotate_attempt3_descr[] = "rotate attempt 3: Let's try separating transpose logic from exchange rows logic. Maybe this will identify any room for optimization.";
void rotate_attempt3(int dim, pixel *src, pixel *dst) {
  //Transpose the array
  int i,j;
  for(i = 0; i < dim; ++i) {
    int src_row = i * dim;
    for(j = 0; j < dim; ++j) {
      int dst_index = j * dim + i;
      int src_index = src_row + j;
      dst[dst_index] = src[src_index];
    }
  }

  //Exchange rows
  unsigned int pixelrowsize = sizeof(pixel)*dim;
  for ( i = 0; i < dim/2; ++i){
    pixel temp[dim];
    int dst_row1 = i * dim;
    int dst_row2 = (dim - 1 - i) * dim;
    memcpy(temp, &dst[dst_row1],pixelrowsize);
    memcpy(&dst[dst_row1], &dst[dst_row2], pixelrowsize);
    memcpy(&dst[dst_row2], temp, pixelrowsize);
  } 
}

char rotate_attempt4_descr[] = "rotate attempt 4. Let's try this magical thing called tiling and split the row sweep into blocks of 4.";
void rotate_attempt4(int dim, pixel *src, pixel *dst) {
  int i, j;
  int dimming = dim*dim-dim;
  for (i = 0; i < dim; i++){
    int dst_row = dimming+i;
    int src_row = i*dim;
    for(j = 0; j < dim/4; j++){
      int dst_index = dst_row - (j*dim);
      int src_index = src_row + j;
      dst[dst_index] = src[src_index];
    } 
  }
  for (i = 0; i < dim; i++){
    int dst_row = dimming+i;
    int src_row = i*dim;
    for(j = dim/4; j < dim/2; j++){
      int dst_index = dst_row - (j*dim);
      int src_index = src_row + j;
      dst[dst_index] = src[src_index];
    } 
  }
  for (i = 0; i < dim; i++){
    int dst_row = dimming+i;
    int src_row = i*dim;
    for(j = dim/2; j < 3*dim/4; j++){
      int dst_index = dst_row - (j*dim);
      int src_index = src_row + j;
      dst[dst_index] = src[src_index];
    } 
  }
  for (i = 0; i < dim; i++){
    int dst_row = dimming+i;
    int src_row = i*dim;
    for(j = 3*dim/4; j < dim; j++){
      int dst_index = dst_row - (j*dim);
      int src_index = src_row + j;
      dst[dst_index] = src[src_index];
    } 
  }
}

char  rotate_attempt5_descr[] = "rotate attempt 5 - pointer arithmetic";

void rotate_attempt5(int dim, pixel *src, pixel *dst) {
    pixel *currsrc = src;
    dst += dim*(dim-1);
    int i, j, k ,l;
    
    int divideDim = dim/BLOCK;
    int dimSquared = dim*dim;
    int dimTimesBlock = dim*BLOCK;
    int dimMinusBlock = dim - BLOCK;
       
    for(i = 0; i < divideDim; i++)
    {
    	for (j = 0; j < divideDim; j++)
    	{
	        for(k = 0; k < BLOCK; k++){   
		        for(l = 0; l < BLOCK; l+=2){
		           *dst = *currsrc;
		           currsrc++;
		           dst -= dim; 
		           
		           *dst = *currsrc;
		           currsrc++;
		           dst -= dim;
		        }
		        
		        currsrc += dimMinusBlock;
		        dst += dimTimesBlock + 1;
	        }
	        
	        currsrc += -dimTimesBlock + BLOCK;
	        dst += -dimTimesBlock - BLOCK;
    	}

    	currsrc += dimTimesBlock - dim;
    	dst += dimSquared + BLOCK;
    }
}

void rotate_attempt6(int dim, pixel *src, pixel *dst) {
   
    int i, j, k ,l;
       
    for(i = 0; j < dim; j+=BLOCK)
    {
    	for (j = 0; i < dim; i+=BLOCK)
    	{
	        for(k = j; (k-j) < BLOCK; k++){   
		        for(l = i; (l-i) < BLOCK; l++){
		          dst[RIDX(dim-1-k, l, dim)] = src[RIDX(l, k, dim)];
		        }
	        }
    	}
    } 
}


/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
     rotate_attempt6(dim, src, dst);
}

/* 
 * second attempt 
*/
char rotate_two_descr[] = "second attempt";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    rotate_attempt2(dim, src, dst);
}

void attempt_three(int dim, pixel *src, pixel *dst){
  rotate_attempt3(dim,src,dst);
}

void attempt_four(int dim, pixel *src, pixel *dst){
  rotate_attempt4(dim,src,dst);
}

void attempt_five(int dim, pixel *src, pixel *dst){
  rotate_attempt5(dim,src,dst);
}



/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   

    //add_rotate_function(&attempt_two, rotate_attempt2_descr);   
    //add_rotate_function(&attempt_three, rotate_attempt3_descr);   
    //add_rotate_function(&attempt_four, rotate_attempt4_descr);   
    //add_rotate_function(&attempt_five, rotate_five_descr);   
    //add_rotate_function(&attempt_six, rotate_six_descr);   
    //add_rotate_function(&attempt_seven, rotate_seven_descr);   
    //add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

