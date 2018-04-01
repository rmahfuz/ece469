#include "usertraps.h"
#include "misc.h"
#define LARGE_NUM 100000

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	int i;
  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 
  
  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now do the actual task
  Printf("part5 (%d): Before counting to the large number\n", getpid());
	for (i = 0; i < LARGE_NUM; i++);
  Printf("part5 (%d): After counting to the large number\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("part5 (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("part5 (%d): Done!\n", getpid());
}
