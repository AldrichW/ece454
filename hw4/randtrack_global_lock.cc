
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "defs.h"
#include "hash.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "team_almost_there",                    /* Team name */

    "Suhaib Ahmed",                         /* First member full name */
    "999054062",                           /* First member student number */
    "suhaib.ahmed@mail.utoronto.ca",        /* First member email address */

    "Aldrich Wingsiong",                    /* Second member full name */
    "998735775",                                     /* Second member student number */
    "aldrich.wingsiong@mail.utoronto.ca"    /* Second member email address */
};

unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample {
  unsigned my_key;
 public:
  sample *next;
  unsigned count;

  sample(unsigned the_key){my_key = the_key; count = 0;};
  unsigned key(){return my_key;}
  void print(FILE *f){printf("%d %d\n",my_key,count);}
};

struct arg_struct {
        int start_index;
        int num_iterations;
};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".  
hash<sample,unsigned> h;
pthread_mutex_t mutex;

void *countSamples(void* arguments){
    int i,j,k;
    int rnum;
    unsigned key;
    sample *s;  
    struct arg_struct args = *(struct arg_struct*)arguments;
    for (i = args.start_index; i < (args.start_index + args.num_iterations); i++){
        rnum = i; 
        // collect a number of samples
        for (j=0; j<SAMPLES_TO_COLLECT; j++){

            // skip a number of samples
            for (k=0; k<samples_to_skip; k++){
	            rnum = rand_r((unsigned int*)&rnum);
            }

            // force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
            key = rnum % RAND_NUM_UPPER_BOUND;
            
            pthread_mutex_lock(&mutex);
            // if this sample has not been counted before
            if (!(s = h.lookup(key))){
	
	        // insert a new element for it into the hash table
	        s = new sample(key);
	        h.insert(s);
            }   

            // increment the count for the sample
            s->count++;
            pthread_mutex_unlock(&mutex);
        }
    }

    free(arguments);

}

int  
main (int argc, char* argv[]){
  int i;

  // Print out team information
  printf( "Team Name: %s\n", team.team );
  printf( "\n" );
  printf( "Student 1 Name: %s\n", team.name1 );
  printf( "Student 1 Student Number: %s\n", team.number1 );
  printf( "Student 1 Email: %s\n", team.email1 );
  printf( "\n" );
  printf( "Student 2 Name: %s\n", team.name2 );
  printf( "Student 2 Student Number: %s\n", team.number2 );
  printf( "Student 2 Email: %s\n", team.email2 );
  printf( "\n" );

  // Parse program arguments
  if (argc != 3){
    printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
    exit(1);  
  }
  sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
  sscanf(argv[2], " %d", &samples_to_skip);

  // initialize a 16K-entry (2**14) hash of empty lists
  h.setup(14);
  int ret = pthread_mutex_init(&mutex, NULL);
  if(ret != 0){
    printf("Something went wrong initializing pthread\n");
    exit(1);
  }
  
  //initialize pthread structure with size num_threads 
  pthread_t *tid = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
  int num_iterations = NUM_SEED_STREAMS/num_threads;
  // process streams starting with different initial numbers
  for (i=0; i<NUM_SEED_STREAMS;){
    struct arg_struct *args = (struct arg_struct*)malloc(sizeof(struct arg_struct));
    args->num_iterations = num_iterations;
    args->start_index = i;
    switch(num_threads){
        case 1:
            if(i == 0){
                pthread_create(&tid[i], NULL, countSamples, (void*)args);
                i+=4;
            }
            break;
        case 2:
            if(i == 0){  
                pthread_create(&tid[i], NULL, countSamples, (void*)args);
            }
            else if(i == 2){
                pthread_create(&tid[1], NULL, countSamples, (void*)args);
            }
            i+=2;
            break;
        case 4:
            pthread_create(&tid[i], NULL, countSamples, (void*)args);
            i++;
            break;
        default:
            break;
    }
  }
  for(i=0; i < num_threads; i++){
    pthread_join(tid[i], NULL);
  }

  // print a list of the frequency of all samples
  h.print();
  free(tid);
}
