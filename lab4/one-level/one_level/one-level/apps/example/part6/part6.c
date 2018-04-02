#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	int* mem_addr = (1<<22);

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 
  
  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

	// Now print a message to show that everything worked
  Printf("part6 (%d): accessing memory beyond maximum virtual address\n", getpid());

  // Since this process will die, signal the semaphore now
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("part6 (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

	//actually accessing that memory
	Printf("part6 (%d): content of memory location %d is <expect illegal access>: \n ", getpid(), mem_addr);
	Printf("%d\n", *mem_addr);

  Printf("part6 (%d): Done!\n", getpid());
}
