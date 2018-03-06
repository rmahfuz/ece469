#include "usertraps.h"
#include "misc.h"

//#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int num_h2o;
  mbox_t h2o_mbox;

  if (argc != 3) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_page_mapped_semaphore> <h2o_mbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  h2o_mbox = dstrtol(argv[2], NULL, 10);

  if(mbox_open(h2o_mbox) != MBOX_SUCCESS){
    Printf("Failed to open mailbox h2o_mbox (%d) in ", h2o_mbox); Printf(argv[0]); //open mbox
    Exit();
  }

  mbox_send(h2o_mbox, 3, (void *) "H2O");   //send message (represents one injection)
  //Printf("H2O sent to mbox\n");

  if(mbox_close(h2o_mbox) != MBOX_SUCCESS){ //close mailbox
    Printf("Failed to close mailbox h2o_mbox (%d) in ", h2o_mbox); Printf(argv[0]);
    Exit();
  }

  Printf("1 H2O injected\n");

  //Printf("spawn_me: PID %d, producer of h2o is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
