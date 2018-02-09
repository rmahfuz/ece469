#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

//#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	int num_four;
	sem_t so4_sem, so2_sem, o2_sem;
  int i; // counter variables

  if (argc != 6) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle to page mapped semaphore> <number of occurences> <so4 semaphore> <so2 semaphore> <o2 semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  num_four = dstrtol(argv[2], NULL, 10);
  so4_sem = dstrtol(argv[3], NULL, 10);
  so2_sem = dstrtol(argv[4], NULL, 10);
  o2_sem = dstrtol(argv[5], NULL, 10);

	for ( i = 0; i < num_four; i++ ) {
		// consuming so4
		if(sem_wait(so4_sem) != SYNC_SUCCESS) {
			Printf("Failed to wait for semaphore so4_sem (%d) in ",so4_sem); Printf(argv[0]);
			Exit();
		}
		// creating so2
		if(sem_signal(so2_sem) != SYNC_SUCCESS) {
			Printf("Failed to signal semaphore so2_sem (%d) in ",so2_sem); Printf(argv[0]);
			Exit();
		}
		// creating o2
		if(sem_signal(o2_sem) != SYNC_SUCCESS) {
			Printf("Failed to signal semaphore o2_sem (%d) in ",o2_sem); Printf(argv[0]);
			Exit();
		}
		Printf("O2 formed from SO4\n");
	}

  //Printf("spawn_me: PID %d, four is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
