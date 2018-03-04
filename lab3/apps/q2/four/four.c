#include "usertraps.h"
#include "misc.h"

//#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	//int num_four;
	char msg[3];
	mbox_t so4_mbox, so2_mbox, o2_mbox;

  if (argc != 5) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle to page mapped semaphore> <so4 mbox> <so2 mbox> <o2 mbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  so4_mbox = dstrtol(argv[2], NULL, 10);
  so2_mbox = dstrtol(argv[3], NULL, 10);
  o2_mbox = dstrtol(argv[4], NULL, 10);

  //open so4 mbox
  if (mbox_open(so4_mbox) != SYNC_SUCCESS){
    Printf("Failed to open mailbox so4_mbox (%d) in ", so4_mbox); Printf(argv[0]);
    Exit();
  }

  //open so2 mbox
  if (mbox_open(so2_mbox) != SYNC_SUCCESS){
    Printf("Failed to open mailbox so2_mbox (%d) in ", so2_mbox); Printf(argv[0]);
    Exit();
  }
  //open o2 mbox
  if (mbox_open(o2_mbox) != SYNC_SUCCESS){
    Printf("Failed to open mailbox o2_mbox (%d) in ", o2_mbox); Printf(argv[0]);
    Exit();
  }

  //recieve 1 so4
  mbox_recv(so4_mbox, 3, (char *) msg);

  //send 1 so2
  mbox_send(so2_mbox, 3, "SO2");
  Printf("Produced 1 SO2\n");

  //send 1 o2
  mbox_send(o2_mbox, 2, "O2");
  Printf("Produced 1 O2\n");

  //close so4 mbox
  if(mbox_close(so4_mbox) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox so4_mbox (%d) in ", so4_mbox); Printf(argv[0]);
    Exit();
  }
  
  //close so2 mbox
  if(mbox_close(so2_mbox) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox so2_mbox (%d) in ", so2_mbox); Printf(argv[0]);
    Exit();
  }

  //close o2 mbox
  if(mbox_close(o2_mbox) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox o2_mbox (%d) in ", o2_mbox); Printf(argv[0]);
    Exit();
  }

  //Printf("Reaction 2 complete!! 1 SO2 and 1 O2 produced\n");

  //Printf("spawn_me: PID %d, four is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
