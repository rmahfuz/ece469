#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	//int num_three;
	mbox_t h2o_mbox, h2_mbox, o2_mbox;
  char msg[3];

  if (argc != 5) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle to page mapped semaphore> <h2o mbox> <h2 mbox> <o2 mbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  h2o_mbox = dstrtol(argv[2], NULL, 10);
  h2_mbox = dstrtol(argv[3], NULL, 10);
  o2_mbox = dstrtol(argv[4], NULL, 10);

  //open h2o mbox
  if (mbox_open(h2o_mbox) != SYNC_SUCCESS){
    Printf("Failed to open mailbox h2o_mbox (%d) in ", h2o_mbox); Printf(argv[0]);
    Exit();
  }

 
  //open h2 mbox
  if (mbox_open(h2_mbox) != SYNC_SUCCESS){
    Printf("Failed to open mailbox h2_mbox (%d) in ", h2_mbox); Printf(argv[0]);
    Exit();
  }

  //open o2 mbox
  if (mbox_open(o2_mbox) != SYNC_SUCCESS){
    Printf("Failed to open mailbox o2_mbox (%d) in ", o2_mbox); Printf(argv[0]);
    Exit();
  }

  //recieve 2 h2o
  mbox_recv(h2o_mbox, 3, (char *) msg);
  mbox_recv(h2o_mbox, 3, (char *) msg);

  //send 2 h2
  mbox_send(h2_mbox, 2, "H2");
  Printf("Produced 1 H2\n");
  mbox_send(h2_mbox, 2, "H2");
  Printf("Produced 1 H2\n");

  //send 1 o2
  mbox_send(o2_mbox, 2, "O2");
  Printf("Produced 1 O2\n");


  //close h2o mbox
  if(mbox_close(h2o_mbox) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox h2o_mbox (%d) in ", h2o_mbox); Printf(argv[0]);
    Exit();
  }

  //close h2 mbox
  if(mbox_close(h2_mbox) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox h2_mbox (%d) in ", h2_mbox); Printf(argv[0]);
    Exit();
  }

  //close o2 mbox
  if(mbox_close(o2_mbox) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox o2_mbox (%d) in ", o2_mbox); Printf(argv[0]);
    Exit();
  }

  //Printf("Reaction 1 complete! 2 H2 and 1 O2 produced\n");
  //Printf("spawn_me: PID %d, three is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
