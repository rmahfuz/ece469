#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
//#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	int num_five;
	sem_t h2_sem, o2_sem, so2_sem, h2so4_sem;
  int i; // counter variables

  if (argc != 7) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle to page mapped semaphore> <number of occurences> <h2 semaphore> <o2 semaphore> <so2 semaphore> <h2so4>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  num_five = dstrtol(argv[2], NULL, 10);
  h2_sem = dstrtol(argv[3], NULL, 10);
  o2_sem = dstrtol(argv[4], NULL, 10);
  so2_sem = dstrtol(argv[5], NULL, 10);
  h2so4_sem = dstrtol(argv[5], NULL, 10);

	for ( i = 0; i < num_five; i++ ) {
		// consuming h2
		if(sem_wait(h2_sem) != SYNC_SUCCESS) {
			Printf("Failed to wait for semaphore h2_sem (%d) in ",h2_sem); Printf(argv[0]);
			Exit();
		}
		// consuming o2
		if(sem_wait(o2_sem) != SYNC_SUCCESS) {
			Printf("Failed to wait for semaphore o2_sem (%d) in ",o2_sem); Printf(argv[0]);
			Exit();
		}
		// consuming so2
		if(sem_wait(so2_sem) != SYNC_SUCCESS) {
			Printf("Failed to wait for semaphore so2_sem (%d) in ",so2_sem); Printf(argv[0]);
			Exit();
		}
		// creating h2so4
		if(sem_signal(h2so4_sem) != SYNC_SUCCESS) {
			Printf("Failed to signal semaphore h2so4_sem (%d) in ",h2so4_sem);Printf(argv[0]);
			Exit();
		}
		Printf("H2SO4 formed\n");
	}

  //Printf("spawn_me: PID %d, five is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
