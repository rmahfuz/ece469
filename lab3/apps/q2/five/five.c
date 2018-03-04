#include "usertraps.h"
#include "misc.h"
//#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	mbox_t h2_mbox, o2_mbox, so2_mbox, h2so4_mbox;
  char hmsg[2], omsg[2], smsg[3];

  if (argc != 6) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle to page mapped semaphore> <h2 mbox> <o2 mbox> <so2 mbox> <h2so4 mbox>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  h2_mbox = dstrtol(argv[2], NULL, 10);
  o2_mbox = dstrtol(argv[3], NULL, 10);
  so2_mbox = dstrtol(argv[4], NULL, 10);
  h2so4_mbox = dstrtol(argv[5], NULL, 10);


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

  //open so2 mbox
  if (mbox_open(so2_mbox) != SYNC_SUCCESS){
    Printf("Failed to open mailbox so2_mbox (%d) in ", so2_mbox); Printf(argv[0]);
    Exit();
  }

  //open h2so4 mbox
  if (mbox_open(h2so4_mbox) != SYNC_SUCCESS){
    Printf("Failed to open mailbox h2so4_mbox (%d) in ", h2so4_mbox); Printf(argv[0]);
    Exit();
  }

  //receive h2 mbox
  mbox_recv(h2_mbox, 2, (char *) hmsg);

  //receive o2 mbox
  mbox_recv(o2_mbox, 2, (char *) omsg);


  //receive so2 mbox
  mbox_recv(so2_mbox, 3, (char *) smsg);

  //send h2so4
  mbox_send(h2so4_mbox, 5, "H2SO4");
  Printf("Produced 1 H2SO4\n");

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

  //close so2 mbox
  if(mbox_close(so2_mbox) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox h2_mbox (%d) in ", so2_mbox); Printf(argv[0]);
    Exit();
  }

  //close h2so4 mbox
  if(mbox_close(h2so4_mbox) != MBOX_SUCCESS) {
    Printf("Failed to close mailbox h2_mbox (%d) in ", h2so4_mbox); Printf(argv[0]);
    Exit();
  }

  //Printf("spawn_me: PID %d, five is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
