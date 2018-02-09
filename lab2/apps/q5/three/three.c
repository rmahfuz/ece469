#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	int num_three;
	sem_t h2o_sem, h2_sem, o2_sem;
  int i, j; // counter variables

  if (argc != 6) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle to page mapped semaphore> <number of occurences> <h2o semaphore> <h2 semaphore> <o2 semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  num_three = dstrtol(argv[2], NULL, 10);
  h2o_sem = dstrtol(argv[3], NULL, 10);
  h2_sem = dstrtol(argv[4], NULL, 10);
  o2_sem = dstrtol(argv[5], NULL, 10);

	for ( i = 0; i < num_three; i++ ) {
		// consuming 2 h2o
		for ( j = 0; j < 2; j++ ) {
			if(sem_wait(h2o_sem) != SYNC_SUCCESS) {
				Printf("Failed to wait for semaphore h2o_sem (%d) in ",h2o_sem); Printf(argv[0]);
				Exit();
			}
		}
		// creating 2 h2
		for ( j = 0; j < 2; j++ ) {
			if(sem_signal(h2_sem) != SYNC_SUCCESS) {
				Printf("Failed to signal semaphore h2_sem (%d) in ",h2_sem); Printf(argv[0]);
				Exit();
			}
		}
		// creating o2
		if(sem_signal(o2_sem) != SYNC_SUCCESS) {
			Printf("Failed to signal semaphore o2_sem (%d) in ",o2_sem); Printf(argv[0]);
			Exit();
		}
		Printf("O2 formed from H2O\n");
  }
 

  //Printf("spawn_me: PID %d, three is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
