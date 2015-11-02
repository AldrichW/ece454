/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

#include <string.h>

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "team_almost_there",              /* Team name */

    "Suhaib Ahmed",     /* First member full name */
    "suhaib.ahmed@mail.utoronto.ca",  /* First member email address */

    "Aldrich Wingsiong",                   /* Second member full name (leave blank if none) */
    "aldrich.wingsiong@mail.utoronto.ca"          /* Second member email addr (leave blank if none) */
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

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j, k, l;
    int block = 32;

    for (i = 0; i < dim; i+=block)
    {
        for (j = 0; j < dim; j+=block)
        {
            for (k = j; k-j < block; k++)
            {
                l = i;
                int first = (dim-1-k)*dim + l;
                int second = l*dim + k;
                dst[first] = src[second];
                dst[first+1] = src[second+dim];
                dst[first+2] = src[second+dim*2];
                dst[first+3] = src[second+dim*3];
                dst[first+4] = src[second+dim*4];
                dst[first+5] = src[second+dim*5];
                dst[first+6] = src[second+dim*6];
                dst[first+7] = src[second+dim*7];
                dst[first+8] = src[second+dim*8];
                dst[first+9] = src[second+dim*9];
                dst[first+10] = src[second+dim*10];
                dst[first+11] = src[second+dim*11];
                dst[first+12] = src[second+dim*12];
                dst[first+13] = src[second+dim*13];
                dst[first+14] = src[second+dim*14];
                dst[first+15] = src[second+dim*15];
                dst[first+16] = src[second+dim*16];
                dst[first+17] = src[second+dim*17];
                dst[first+18] = src[second+dim*18];
                dst[first+19] = src[second+dim*19];
                dst[first+20] = src[second+dim*20];
                dst[first+21] = src[second+dim*21];
                dst[first+22] = src[second+dim*22];
                dst[first+23] = src[second+dim*23];
                dst[first+24] = src[second+dim*24];
                dst[first+25] = src[second+dim*25];
                dst[first+26] = src[second+dim*26];
                dst[first+27] = src[second+dim*27];
                dst[first+28] = src[second+dim*28];
                dst[first+29] = src[second+dim*29];
                dst[first+30] = src[second+dim*30];
                dst[first+31] = src[second+dim*31];
            }
        }
    }
}


/* second attempt */
char rotate_two_descr[] = "second attempt";
void attempt_two(int dim, pixel *src, pixel *dst)
{
    int block = 8;
    dst += dim*(dim-1);
    int i, j, k ,l;
    
    int divideDim = dim/block;
    int dimSquared = dim*dim;
    int dimTimesBlock = dim*block;
    int dimMinusBlock = dim - block;
       
    for(i = 0; i < divideDim; i++)
    {
        for (j = 0; j < divideDim; j++)
        {
            for(k = 0; k < block; k++)
            {
                for(l = 0; l < block; l+=2)
                {
                   *dst = *src;
                   src++;
                   dst -= dim;
                   
                   *dst = *src;
                   src++;
                   dst -= dim;
                }
                
                src += dimMinusBlock;
                dst += dimTimesBlock + 1;
            }
            
            src += -dimTimesBlock + block;
            dst += -dimTimesBlock - block;
        }

        src += dimTimesBlock - dim;
        dst += dimSquared + block;
    }
}


/* third attempt */
char rotate_three_descr[] = "third attempt: separating transpose logic from exchange rows logic";
void attempt_three(int dim, pixel *src, pixel *dst)
{
    //Transpose the array
    int i,j;
    for(i = 0; i < dim; ++i)
    {
        int src_row = i * dim;
        for(j = 0; j < dim; ++j)
	{
            int dst_index = j * dim + i;
            int src_index = src_row + j;
            dst[dst_index] = src[src_index];
        }
    }

    //Exchange rows
    unsigned int pixelrowsize = sizeof(pixel)*dim;
    for ( i = 0; i < dim/2; ++i)
    {
        pixel temp[dim];
        int dst_row1 = i * dim;
        int dst_row2 = (dim - 1 - i) * dim;
        memcpy(temp, &dst[dst_row1],pixelrowsize);
        memcpy(&dst[dst_row1], &dst[dst_row2], pixelrowsize);
        memcpy(&dst[dst_row2], temp, pixelrowsize);
    } 
}


/* fourth attempt */
char rotate_four_descr[] = "fourth attempt: Simple tiling";
void attempt_four(int dim, pixel *src, pixel *dst)
{
   
    int i, j, k ,l;
    int block = 32;
       
    for(i = 0; i < dim; i+=block)
    {
        for (j = 0; j < dim; j+=block)
        {
            for(k = i; (k-i) < block; k++)
	    {   
                for(l = j; (l-j) < block; l++)
		{
                    dst[RIDX(dim-1-l, k, dim)] = src[RIDX(k, l, dim)];
                }
            }
        }
    } 
}

/* fifth attempt */
char rotate_five_descr[] = "fifth attempt: Tiling with column traversal in tile (for src)";
void attempt_five(int dim, pixel *src, pixel *dst)
{
    int i, j, k, l;
    int block = 32;

    for (i = 0; i < dim; i+=block)
    {
        for (j = 0; j < dim; j+=block)
        {
            for (k = j; k-j < block; k++)
            {
                for (l = i; l-i < block; l++)
                {
                    dst[(dim-1-k)*dim + l] = src[l*dim + k];
                }
            }
        }
    }
}

/* sixth attempt */
char rotate_six_descr[] = "sixth attempt: Tiling with column traversal in tile (for src), and loop unrolling";
void attempt_six(int dim, pixel *src, pixel *dst)
{
    int i, j, k, l;
    int block = 32;

    for (i = 0; i < dim; i+=block)
    {
        for (j = 0; j < dim; j+=block)
        {
            for (k = j; k-j < block; k++)
            {
            l = i;
                int first = (dim-1-k)*dim + l;
                int second = l*dim + k;
                dst[first++] = src[second];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first++] = src[second+=dim];
                dst[first] = src[second+=dim];
            }
        }
    }
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

    add_rotate_function(&attempt_two, rotate_two_descr);   
    add_rotate_function(&attempt_three, rotate_three_descr);   
    add_rotate_function(&attempt_four, rotate_four_descr);   
    add_rotate_function(&attempt_five, rotate_five_descr);   
    add_rotate_function(&attempt_six, rotate_six_descr);   
    //add_rotate_function(&attempt_seven, rotate_seven_descr);   
    //add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

