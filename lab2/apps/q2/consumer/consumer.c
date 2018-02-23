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
  //int prev_head;        // Previous head stored when inserting into buffer
  char val_read;        // Character that was read from circular buffer
  //char to_write[] = "Hello world"; // String to write into buffer
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
		while (cb->head == cb->tail); // wait while buffer is empty
		// get the lock
		if (lock_acquire(lock_buf) == SYNC_FAIL) {
			Printf("Could not acquire lock in "); Printf(argv[0]); Printf("\n");
			Exit();
		}
		// read character from the circular buffer
		val_read = cb->buf[cb->tail];
		cb->tail = (cb->tail + 1)%BUF_SIZE;
		// release the lock
	    //Printf("before releasing: Consumer %d removed: %c, head = %d, tail = %d\n", getpid(), val_read, cb->head, cb->tail);
	    Printf("Consumer %d removed: %c\n", getpid(), val_read);
		if (lock_release(lock_buf) == SYNC_FAIL) {
			Printf("Could not release lock in "); Printf(argv[0]); Printf("\n");
			Exit();
		}
	    //Printf("Consumer %d removed: %c, head = %d, tail = %d\n", getpid(), val_read, cb->head, cb->tail);
	    //Printf("Consumer %d removed: %c\n", getpid(), val_read);
  }

 
  // Now print a message to show that everything worked
  //Printf("producer: This is one of the %d producer instances you created.  ", cb->numprocs);
  //Printf("producer: Missile code is: %c\n", cb->really_important_char);
  //Printf("producer: My PID is %d\n", getpid());


  // Signal the semaphore to tell the original process that we're done
  //Printf("consumer: PID %d is complete, head = %d, tail = %d\n", getpid(), cb->head, cb->tail);
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
