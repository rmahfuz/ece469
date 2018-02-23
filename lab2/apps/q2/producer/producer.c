#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
//#include "string.h"
#define STRING_LEN 11

#include "spawn.h"

void main (int argc, char *argv[])
{
  circ_buf *cb;        // Used to access circular buffer in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int prev_head;        // Previous head stored when inserting into buffer
  //char to_insert;      // Character to insert into buffer
  char to_write[] = "Hello world"; // String to write into buffer
  int i;               // Counter variable
  lock_t lock_buf;     // Lock to modify circular buffer

  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <lock_for_circular_buffer>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  lock_buf = dstrtol(argv[3], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((cb = (circ_buf *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  for (i = 0; i < STRING_LEN; i++) {
   //Printf("");
    while ((cb->head+1)%BUF_SIZE == cb->tail); // wait while buffer is full
		// get the lock
		if (lock_acquire(lock_buf) == SYNC_FAIL) {
			Printf("Could not acquire lock in "); Printf(argv[0]); Printf("\n");
			Exit();
		}
		// write  character from "Hello World" into the circular buffer
		//prev_head = cb->head;
		cb->buf[cb->head] = to_write[i];
		cb->head = (cb->head+1)%BUF_SIZE;
		//cb->buf[prev_head] = to_write[i];
	    Printf("Producer %d inserted: %c\n", getpid(), to_write[i]);
		// release the lock
		if (lock_release(lock_buf) == SYNC_FAIL) {
			Printf("Could not release lock in "); Printf(argv[0]); Printf("\n");
			Exit();
		}
	  //Printf("Producer %d inserted: %c, head = %d, tail = %d\n", getpid(), to_write[i], cb->head, cb->tail);
	  //Printf("Producer %d inserted: %c\n", getpid(), to_write[i]);
}

 
  // Now print a message to show that everything worked
  //Printf("producer: This is one of the %d producer instances you created.  ", cb->numprocs);
  //Printf("producer: Missile code is: %c\n", cb->really_important_char);
  //Printf("producer: My PID is %d\n", getpid());


  // Signal the semaphore to tell the original process that we're done
  //Printf("producer: PID %d is complete, head = %d, tail = %d\n", getpid(), cb->head, cb->tail);
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
