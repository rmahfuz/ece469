#include "usertraps.h"
#include "misc.h"

//#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  mbox_t so4_mbox;

  if (argc != 3) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_page_mapped_semaphore> <so4_mbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  so4_mbox = dstrtol(argv[2], NULL, 10);

  if(mbox_open(so4_mbox) != MBOX_SUCCESS){
    Printf("Failed to open mailbox so4_mbox (%d) in ", so4_mbox); Printf(argv[0]);
    Exit();
  }

  mbox_send(so4_mbox, 3, (void *) "SO4");
  //Printf("SO4 sent to mbox\n");

  if(mbox_close(so4_mbox) != MBOX_SUCCESS){
    Printf("Failed to close mailbox so4_mbox (%d) in ", so4_mbox); Printf(argv[0]);
    Exit();
  }

  Printf("1 S04 injected\n");

  //Printf("spawn_me: PID %d, producer of so4 is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
