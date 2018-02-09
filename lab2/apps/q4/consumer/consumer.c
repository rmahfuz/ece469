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
  int pool_was_full = 0;
  cond_t NotFULL, NotEMPTY; // condition variables passed as command line arguments

  if (argc != 6) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <condition_variable_NotFULL> <condition_variable_NotEMPTY> <lock_to_access_circular_buffer>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  NotFULL = dstrtol(argv[3], NULL, 10);
  NotEMPTY = dstrtol(argv[4], NULL, 10);
  lock_buf = dstrtol(argv[5], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((cb = (circ_buf *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  for (i = 0; i < STRING_LEN; i++) {
	// acquire the mutex
    if (lock_acquire(lock_buf) == SYNC_FAIL) {
		Printf("Could not acquire lock in "); Printf(argv[0]); Printf("\n");
		Exit();
	}
    if (cb->head == cb->tail) { // if pool is empty
		// release the mutex
	/*	if (lock_release(lock_buf) == SYNC_FAIL) {
			Printf("Could not release lock in "); Printf(argv[0]); Printf("\n");
			Exit();
		}*/
		// wait for NotEMPTY
		if (cond_wait(NotEMPTY) != 0) {
	      Printf("Could not wait for condition variable %d NotEMPTY in ", NotEMPTY); Printf(argv[0]); Printf("\n");
		  Exit();
        }
		// acquire the mutex
		if (lock_acquire(lock_buf) == SYNC_FAIL) {
			Printf("Could not acquire lock in "); Printf(argv[0]); Printf("\n");
			Exit();
		}
	}
	// record if pool was full
	if ((cb->head+1)%BUF_SIZE == cb->tail)
		pool_was_full = 1;

	// remove the item
	val_read = cb->buf[cb->tail];
	cb->tail = (cb->tail + 1)%BUF_SIZE;
	if (pool_was_full) { // if pool was full
		if (cond_signal(NotFULL) != 0) { // signal NotFULL
	      Printf("Could not signal condition variable %d NotFULL in ", NotFULL); Printf(argv[0]); Printf("\n");
		  Exit();
		}
		pool_was_full = 0;
	}

	// release the mutex
	if (lock_release(lock_buf) == SYNC_FAIL) {
		Printf("Could not release lock in "); Printf(argv[0]); Printf("\n");
		Exit();
	}
	// consume the item
	Printf("Consumer %d removed: %c\n", getpid(), val_read);
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
