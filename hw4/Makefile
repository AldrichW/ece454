
CC = g++-4.9
CFLAGS = -O3 -lpthread -fgnu-tm

all: randtrack randtrack_parallel randtrack_global_lock

randtrack: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) randtrack.cc -o randtrack

randtrack_parallel: list.h hash.h defs.h randtrack_parallel.cc
	$(CC) $(CFLAGS) randtrack_parallel.cc -o randtrack_parallel

randtrack_global_lock: list.h hash.h defs.h randtrack_global_lock.cc
	$(CC) $(CFLAGS) randtrack_global_lock.cc -o randtrack_global_lock

clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock
