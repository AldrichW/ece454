/*******************************************************************
 * 
 * driver.c - Driver program for CS:APP Performance Lab
 * 
 * In kernels.c, students generate an arbitrary number of rotate 
 * test functions, which they then register with the driver
 * program using the add_rotate_function()
 * function.
 * 
 * The driver program runs and measures the registered test functions
 * and reports their performance.
 * 
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights
 * reserved.  May not be used, modified, or copied without permission.
 *
 ********************************************************************/

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "fcyc.h"
#include "defs.h"
#include "config.h"

/* Team structure that identifies the students */
extern team_t team; 

/* Keep track of a number of different test functions */
#define MAX_BENCHMARKS 100
#define DIM_CNT 7

/* Misc constants */
#define BSIZE 32     /* cache block size in bytes */     
//#define MAX_DIM 1280 /* 1024 + 256 */
#define MAX_DIM 4352 /* 16384 + 256 */
#define ODD_DIM 96   /* not a power of 2 */

/* fast versions of min and max */
#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

/* This struct characterizes the results for one benchmark test */
typedef struct {
    lab_test_func tfunct; /* The test function */
    double cpes[DIM_CNT]; /* One CPE result for each dimension */
    char *description;    /* ASCII description of the test function */
    unsigned short valid; /* The function is tested if this is non zero */
} bench_t;

/* The range of image dimensions that we will be testing */
//static int test_dim_rotate[] = {64, 128, 256, 512, 1024, };
static int test_dim_rotate[] = {64, 128, 256, 512, 1024,2048,4096};


/* Baseline CPEs (see config.h) */
static double rotate_baseline_cpes[] = {R64, R128, R256, R512, R1024, R2048,R4096,R8192};

/* These hold the results for all benchmarks */
static bench_t benchmarks_rotate[MAX_BENCHMARKS];

/* These give the sizes of the above lists */
static int rotate_benchmark_count = 0;

/* 
 * An image is a dimxdim matrix of pixels stored in a 1D array.  The
 * data array holds three images (the input original, a copy of the original, 
 * and the output result array. There is also an additional BSIZE bytes
 * of padding for alignment to cache block boundaries.
 */
static pixel data[(3*MAX_DIM*MAX_DIM) + (BSIZE/sizeof(pixel))];

/* Various image pointers */
static pixel *orig = NULL;         /* original image */
static pixel *copy_of_orig = NULL; /* copy of original for checking result */
static pixel *result = NULL;       /* result image */

/* Keep track of the best rotate score for grading */
double rotate_maxmean = 0.0;
char *rotate_maxmean_desc = NULL;

/******************** Functions begin *************************/

void add_rotate_function(lab_test_func f, char *description) 
{
    benchmarks_rotate[rotate_benchmark_count].tfunct = f;
    benchmarks_rotate[rotate_benchmark_count].description = description;
    benchmarks_rotate[rotate_benchmark_count].valid = 0;
    rotate_benchmark_count++;
}

/* 
 * random_in_interval - Returns random integer in interval [low, high) 
 */
static int random_in_interval(int low, int high) 
{
    int size = high - low;
    return (rand()% size) + low;
}

/*
 * create - creates a dimxdim image aligned to a BSIZE byte boundary
 */
static void create(int dim)
{
    int i, j;

    /* Align the images to BSIZE byte boundaries */
    orig = data;
    while ((unsigned long)orig % BSIZE)
	orig = (pixel *) ((char *)orig + 1);
    result = orig + dim*dim;
    copy_of_orig = result + dim*dim;

    for (i = 0; i < dim; i++) {
	for (j = 0; j < dim; j++) {
	    /* Original image initialized to random colors */
	    orig[RIDX(i,j,dim)].red = random_in_interval(0, 65536);
	    orig[RIDX(i,j,dim)].green = random_in_interval(0, 65536);
	    orig[RIDX(i,j,dim)].blue = random_in_interval(0, 65536);

	    /* Copy of original image for checking result */
	    copy_of_orig[RIDX(i,j,dim)].red = orig[RIDX(i,j,dim)].red;
	    copy_of_orig[RIDX(i,j,dim)].green = orig[RIDX(i,j,dim)].green;
	    copy_of_orig[RIDX(i,j,dim)].blue = orig[RIDX(i,j,dim)].blue;

	    /* Result image initialized to all black */
	    result[RIDX(i,j,dim)].red = 0;
	    result[RIDX(i,j,dim)].green = 0;
	    result[RIDX(i,j,dim)].blue = 0;
	}
    }

    return;
}


/* 
 * compare_pixels - Returns 1 if the two arguments don't have same RGB
 *    values, 0 o.w.  
 */
static int compare_pixels(pixel p1, pixel p2) 
{
    return 
	(p1.red != p2.red) || 
	(p1.green != p2.green) || 
	(p1.blue != p2.blue);
}


/* Make sure the orig array is unchanged */
static int check_orig(int dim) 
{
    int i, j;

    for (i = 0; i < dim; i++) 
	for (j = 0; j < dim; j++) 
	    if (compare_pixels(orig[RIDX(i,j,dim)], copy_of_orig[RIDX(i,j,dim)])) {
		printf("\n");
		printf("Error: Original image has been changed!\n");
		return 1;
	    }

    return 0;
}

/* 
 * check_rotate - Make sure the rotate actually works. 
 * The orig array should not  have been tampered with! 
 */
static int check_rotate(int dim) 
{
    int err = 0;
    int i, j;
    int badi = 0;
    int badj = 0;
    pixel orig_bad = {0}, res_bad = {0};

    /* return 1 if the original image has been  changed */
    if (check_orig(dim)) 
	return 1; 

    for (i = 0; i < dim; i++) 
	for (j = 0; j < dim; j++) 
	    if (compare_pixels(orig[RIDX(i,j,dim)], 
			       result[RIDX(dim-1-j,i,dim)])) {
		err++;
		badi = i;
		badj = j;
		orig_bad = orig[RIDX(i,j,dim)];
		res_bad = result[RIDX(dim-1-j,i,dim)];
	    }

    if (err) {
	printf("\n");
	printf("ERROR: Dimension=%d, %d errors\n", dim, err);    
	printf("E.g., The following two pixels should have equal value:\n");
	printf("src[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       badi, badj, orig_bad.red, orig_bad.green, orig_bad.blue);
	printf("dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       (dim-1-badj), badi, res_bad.red, res_bad.green, res_bad.blue);
    }

    return err;
}

#if 0
static pixel check_average(int dim, int i, int j, pixel *src) {
    pixel result;
    int num = 0;
    int ii, jj;
    int sum0, sum1, sum2;
    int top_left_i, top_left_j;
    int bottom_right_i, bottom_right_j;

    top_left_i = max(i-1, 0);
    top_left_j = max(j-1, 0);
    bottom_right_i = min(i+1, dim-1); 
    bottom_right_j = min(j+1, dim-1);

    sum0 = sum1 = sum2 = 0;
    for(ii=top_left_i; ii <= bottom_right_i; ii++) {
	for(jj=top_left_j; jj <= bottom_right_j; jj++) {
	    num++;
	    sum0 += (int) src[RIDX(ii,jj,dim)].red;
	    sum1 += (int) src[RIDX(ii,jj,dim)].green;
	    sum2 += (int) src[RIDX(ii,jj,dim)].blue;
	}
    }
    result.red = (unsigned short) (sum0/num);
    result.green = (unsigned short) (sum1/num);
    result.blue = (unsigned short) (sum2/num);
 
    return result;
}
#endif


void func_wrapper(void *arglist[]) 
{
    pixel *src, *dst;
    int mydim;
    lab_test_func f;

    f = (lab_test_func) arglist[0];
    mydim = *((int *) arglist[1]);
    src = (pixel *) arglist[2];
    dst = (pixel *) arglist[3];

    (*f)(mydim, src, dst);

    return;
}

void run_rotate_benchmark(int idx, int dim) 
{
    benchmarks_rotate[idx].tfunct(dim, orig, result);
}

void test_rotate(int bench_index) 
{
    int i;
    int test_num;
    char *description = benchmarks_rotate[bench_index].description;
  
    for (test_num = 0; test_num < DIM_CNT; test_num++) {
	int dim;

	/* Check for odd dimension */
	create(ODD_DIM);
	run_rotate_benchmark(bench_index, ODD_DIM);
	if (check_rotate(ODD_DIM)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_rotate[bench_index].description, ODD_DIM);
	    return;
	}

	/* Create a test image of the required dimension */
	dim = test_dim_rotate[test_num];
	create(dim);
#ifdef DEBUG
	printf("DEBUG: Running benchmark \"%s\"\n", benchmarks_rotate[bench_index].description);
#endif

	/* Check that the code works */
	run_rotate_benchmark(bench_index, dim);
	if (check_rotate(dim)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_rotate[bench_index].description, dim);
	    return;
	}

	/* Measure CPE */
	{
	    double num_cycles, cpe;
	    int tmpdim = dim;
	    void *arglist[4];
	    double dimension = (double) dim;
	    double work = dimension*dimension;
#ifdef DEBUG
	    printf("DEBUG: dimension=%.1f\n",dimension);
	    printf("DEBUG: work=%.1f\n",work);
#endif
	    arglist[0] = (void *) benchmarks_rotate[bench_index].tfunct;
	    arglist[1] = (void *) &tmpdim;
	    arglist[2] = (void *) orig;
	    arglist[3] = (void *) result;

	    create(dim);
	    num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist); 
	    cpe = num_cycles/work;
	    benchmarks_rotate[bench_index].cpes[test_num] = cpe;
	}
    }

    /* 
     * Print results as a table 
     */
    printf("Rotate: Version = %s:\n", description);
    printf("Dim\t");
    for (i = 0; i < DIM_CNT; i++)
	printf("\t%d", test_dim_rotate[i]);
    printf("\tMean\n");
  
    printf("Your CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", benchmarks_rotate[bench_index].cpes[i]);
    }
    printf("\n");

    printf("Baseline CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", rotate_baseline_cpes[i]);
    }
    printf("\n");

    /* Compute Speedup */
    {
	double prod, ratio, mean;
	prod = 1.0; /* Geometric mean */
	printf("Speedup\t");
	for (i = 0; i < DIM_CNT; i++) {
	    if (benchmarks_rotate[bench_index].cpes[i] > 0.0) {
		ratio = rotate_baseline_cpes[i]/
		    benchmarks_rotate[bench_index].cpes[i];
	    }
	    else {
		printf("Fatal Error: Non-positive CPE value...\n");
		exit(EXIT_FAILURE);
	    }
	    prod *= ratio;
	    printf("\t%.1f", ratio);
	}

	/* Geometric mean */
	mean = pow(prod, 1.0/(double) DIM_CNT);
	printf("\t%.1f", mean);
	printf("\n\n");
	if (mean > rotate_maxmean) {
	    rotate_maxmean = mean;
	    rotate_maxmean_desc = benchmarks_rotate[bench_index].description;
	}
    }


#ifdef DEBUG
    fflush(stdout);
#endif
    return;  
}

void usage(char *progname) 
{
    fprintf(stderr, "Usage: %s [-hqg] [-f <func_file>] [-d <dump_file>]\n", progname);    
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h         Print this message\n");
    fprintf(stderr, "  -q         Quit after dumping (use with -d )\n");
    fprintf(stderr, "  -g         Autograder mode: checks only rotate()\n");
    fprintf(stderr, "  -f <file>  Get test function names from dump file <file>\n");
    fprintf(stderr, "  -d <file>  Emit a dump file <file> for later use with -f\n");
    exit(EXIT_FAILURE);
}



int main(int argc, char *argv[])
{
    int i;
    int quit_after_dump = 0;
    int skip_teamname_check = 0;
    int autograder = 0;
    int seed = 1729;
    char c = '0';
    char *bench_func_file = NULL;
    char *func_dump_file = NULL;

    /* register all the defined functions */
    register_rotate_functions();

    /* parse command line args */
    while ((c = getopt(argc, argv, "tgqf:d:s:h")) != -1)
	switch (c) {

	case 't': /* skip team name check (hidden flag) */
	    skip_teamname_check = 1;
	    break;

	case 's': /* seed for random number generator (hidden flag) */
	    seed = atoi(optarg);
	    break;

	case 'g': /* autograder mode (checks only rotate()) */
	    autograder = 1;
	    break;

	case 'q':
	    quit_after_dump = 1;
	    break;

	case 'f': /* get names of benchmark functions from this file */
	    bench_func_file = strdup(optarg);
	    break;

	case 'd': /* dump names of benchmark functions to this file */
	    func_dump_file = strdup(optarg);
	    {
		int i;
		FILE *fp = fopen(func_dump_file, "w");	

		if (fp == NULL) {
		    printf("Can't open file %s\n",func_dump_file);
		    exit(-5);
		}

		for(i = 0; i < rotate_benchmark_count; i++) {
		    fprintf(fp, "R:%s\n", benchmarks_rotate[i].description); 
		}
		fclose(fp);
	    }
	    break;

	case 'h': /* print help message */
	    usage(argv[0]);

	default: /* unrecognized argument */
	    usage(argv[0]);
	}

    if (quit_after_dump) 
	exit(EXIT_SUCCESS);


    /* Print team info */
    if (!skip_teamname_check) {
	if (strcmp("bovik", team.team) == 0) {
	    printf("%s: Please fill in the team struct in kernels.c.\n", argv[0]);
	    exit(1);
	}
	printf("Teamname: %s\n", team.team);
	printf("Member 1: %s\n", team.name1);
	printf("Email 1: %s\n", team.email1);
	if (*team.name2 || *team.email2) {
	    printf("Member 2: %s\n", team.name2);
	    printf("Email 2: %s\n", team.email2);
	}
	printf("\n");
    }

    srand(seed);

    /* 
     * If we are running in autograder mode, we will only test
     * the rotate() and bench() functions.
     */
    if (autograder) {
	rotate_benchmark_count = 1;

	benchmarks_rotate[0].tfunct = rotate;
	benchmarks_rotate[0].description = "rotate() function";
	benchmarks_rotate[0].valid = 1;
    }

    /* 
     * If the user specified a file name using -f, then use
     * the file to determine the versions of rotate to test
     */
    else if (bench_func_file != NULL) {
	char flag;
	char func_line[256];
	FILE *fp = fopen(bench_func_file, "r");

	if (fp == NULL) {
	    printf("Can't open file %s\n",bench_func_file);
	    exit(-5);
	}
    
	while(func_line == fgets(func_line, 256, fp)) {
	    char *func_name = func_line;
	    char **strptr = &func_name;
	    char *token = strsep(strptr, ":");
	    flag = token[0];
	    func_name = strsep(strptr, "\n");
#ifdef DEBUG
	    printf("Function Description is %s\n",func_name);
#endif

	    if (flag == 'R') {
		for(i=0; i<rotate_benchmark_count; i++) {
		    if (strcmp(benchmarks_rotate[i].description, func_name) == 0)
			benchmarks_rotate[i].valid = 1;
		}
	    }
	}

	fclose(fp);
    }

    /* 
     * If the user didn't specify a dump file using -f, then 
     * test all of the functions
     */
    else { /* set all valid flags to 1 */
	for (i = 0; i < rotate_benchmark_count; i++)
	    benchmarks_rotate[i].valid = 1;
    }

    /* Set measurement (fcyc) parameters */
    set_fcyc_cache_size(1 << 14); /* 16 KB cache size */
    set_fcyc_clear_cache(1); /* clear the cache before each measurement */
    set_fcyc_compensate(1); /* try to compensate for timer overhead */
 
    for (i = 0; i < rotate_benchmark_count; i++) {
	if (benchmarks_rotate[i].valid)
	    test_rotate(i);
    
}


    if (autograder) {
	printf("\nbestscores:%.1f\n", rotate_maxmean);
    }
    else {
	printf("Summary of Your Best Scores:\n");
	printf("  Rotate: %3.1f (%s)\n", rotate_maxmean, rotate_maxmean_desc);
    }

    return 0;
}













