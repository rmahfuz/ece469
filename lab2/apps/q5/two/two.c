#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

//#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int num_so4;
  sem_t so4_sem;
  int i;

  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_page_mapped_semaphore> <number of so4 molecules> <so4 semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  num_so4 = dstrtol(argv[2], NULL, 10);
  so4_sem = dstrtol(argv[3], NULL, 10);

	for ( i = 0; i < num_so4; i++ ) {
		// create an so4 molecule
		if(sem_signal(so4_sem) != SYNC_SUCCESS) {
			Printf("Failed to signal semaphore so4_sem (%d) in ", so4_sem); Printf(argv[0]);
			Exit();
		}
    Printf("SO4 injected\n");
  }

  //Printf("spawn_me: PID %d, two is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
