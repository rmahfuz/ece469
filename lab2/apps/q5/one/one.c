#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

//#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int num_h2o;
  sem_t h2o_sem;
  int i;

  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_page_mapped_semaphore> <number of h2o molecules> <h2o semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  num_h2o = dstrtol(argv[2], NULL, 10);
  h2o_sem = dstrtol(argv[3], NULL, 10);

  for ( i = 0; i < num_h2o; i++ ) {
    // create an h2o molecule
		if(sem_signal(h2o_sem) != SYNC_SUCCESS) {
			Printf("Failed to signal semaphore h2o_sem (%d) in ", h2o_sem); Printf(argv[0]);
			Exit();
		}
    Printf("H2O injected\n");
  } 

  //Printf("spawn_me: PID %d, producer of h2o is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
