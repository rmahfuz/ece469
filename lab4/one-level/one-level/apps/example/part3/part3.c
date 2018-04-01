#include "usertraps.h"
#include "misc.h"

int grow(int n) {  // to grow the stack
	if (!n) return 0;
	return (grow(n-1) + 1);
}


void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	int* mem_addr = (1<<22) - (1<<12) - 4;

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 
  
  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

	// Now print a message to show that everything worked
  Printf("part4 (%d): cause the user function call stack to grow larger than one page\n", getpid());

	//actually growing the call stack
	grow(2510);

  // Signal the semaphore to tell the origibal process that we are done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("part4 (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("part4 (%d): Done!\n", getpid());
}

