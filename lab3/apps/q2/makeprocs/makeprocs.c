//i#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

//#include "spawn.h"

int findmin(int a, int b, int c) {
	int ans = a;
	if (b < ans) ans = b;
	if (c < ans) ans = c;
	return ans;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main (int argc, char *argv[])
{
  int num_h2o, num_so4;           // Numbers of molecules to be injected
  int num_three, num_four, num_five; //Numbers of each reaction
  int num;                          // Loop index variable
  //missile_code *mc;               // Used to get address of shared memory page
  //uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  mbox_t h2o_mbox, h2_mbox, o2_mbox, so4_mbox, so2_mbox, h2so4_mbox;
  char h2o_mbox_str[10], h2_mbox_str[10], o2_mbox_str[10], so4_mbox_str[10], so2_mbox_str[10], h2so4_mbox_str[10];
  //char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char num_one_str[10], num_two_str[10], num_three_str[10], num_four_str[10], num_five_str[10];

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of H20 molecules> <number of SO4 molecules>\n");
    Exit();
  }

  // command line argument to int
  num_h2o = dstrtol(argv[1], NULL, 10); // number of h2o injected
  num_so4 = dstrtol(argv[2], NULL, 10); // the "10" means base 10

  num_three = (int) (num_h2o / 2);    //number of times reaction three occurs (calculated from equations)
  num_four = (int) (num_so4);         //number of times reaction four occcurs
  num_five = (int) (findmin(num_h2o, 0.5*num_h2o + num_so4, num_so4));  //number of times reaction five occurs
  // int to command line argument

  ditoa(0.5*num_h2o, num_three_str);
  ditoa(num_so4, num_four_str);
  ditoa(findmin(num_h2o, 0.5*num_h2o + num_so4, num_so4), num_five_str);
  
  // Create semaphore to not exit this process until all other processes are done
  if ((s_procs_completed = sem_create(-((num_h2o + num_so4 + num_three + num_four + num_five)-1))) == SYNC_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(s_procs_completed, s_procs_completed_str);
  //Creating mboxes==============================================================================================================
  if ((h2o_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(h2o_mbox, h2o_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((h2_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(h2_mbox, h2_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((o2_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(o2_mbox, o2_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((so4_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(so4_mbox, so4_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((so2_mbox = mbox_create(0)) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(so2_mbox, so2_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((h2so4_mbox = mbox_create(0)) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(h2so4_mbox, h2so4_mbox_str);
  //==============================================================================================================

  
  //Opening Mailboxes==============================================================================================================
   
  if (mbox_open(h2o_mbox) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(h2o_mbox, h2o_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((mbox_open(h2_mbox)) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(h2_mbox, h2_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((mbox_open(o2_mbox)) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(o2_mbox, o2_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if (mbox_open(so4_mbox) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(so4_mbox, so4_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if (mbox_open(so2_mbox) == MBOX_FAIL) {

    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(so2_mbox, so2_mbox_str);
  //---------------------------------------------------------------------------------------------------------------
  if (mbox_open(h2so4_mbox) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(h2so4_mbox, h2so4_mbox_str);
  //==============================================================================================================


 /* // creating processes (from lab 2)
  process_create("one.dlx.obj",   s_procs_completed_str, argv[1], h2o_mbox_str, NULL);  // producer of h2o
  process_create("two.dlx.obj",   s_procs_completed_str, argv[2], so4_mbox_str, NULL);  // producer of so4
  process_create("three.dlx.obj", s_procs_completed_str, num_three_str, h2o_mbox_str, h2_mbox_str, o2_mbox_str,  NULL);  // consumer of 2h20
  process_create("four.dlx.obj",  s_procs_completed_str, num_four_str, so4_mbox_str, so2_mbox_str, o2_mbox_str,   NULL);  // consumer of so4
  process_create("five.dlx.obj",  s_procs_completed_str, num_five_str, h2_mbox_str, o2_mbox_str, so2_mbox_str, h2so4_mbox_str, NULL);  // consumer of h2, o2 and so2
*/

  //creating processes
  for (num =0; num < num_h2o; num++){   //one process for each injection
    process_create("one.dlx.obj", 0, 0, s_procs_completed_str, h2o_mbox_str);
  }

  for (num =0; num < num_so4; num++){
    process_create("two.dlx.obj", 0,0, s_procs_completed_str, so4_mbox_str);
  }

  for (num =0; num < num_three; num++){ //one process for each reaction
    process_create("three.dlx.obj", 0,0, s_procs_completed_str, h2o_mbox_str, h2_mbox_str, o2_mbox_str);
  }

  for (num =0; num < num_four; num++){
    process_create("four.dlx.obj", 0,0, s_procs_completed_str, so4_mbox_str, so2_mbox_str, o2_mbox_str);
  }
  
  for (num =0; num < num_five; num++){
    process_create("five.dlx.obj", 0,0, s_procs_completed_str, h2_mbox_str, o2_mbox_str, so2_mbox_str, h2so4_mbox_str);
  }

 

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }

  
  //Closing Mailboxes==============================================================================================================
   
  if (mbox_close(h2o_mbox) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //---------------------------------------------------------------------------------------------------------------
  if ((mbox_close(h2_mbox)) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //---------------------------------------------------------------------------------------------------------------
  if ((mbox_close(o2_mbox)) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //---------------------------------------------------------------------------------------------------------------
  if (mbox_close(so4_mbox) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //---------------------------------------------------------------------------------------------------------------
  if (mbox_close(so2_mbox) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //---------------------------------------------------------------------------------------------------------------
  if (mbox_close(h2so4_mbox) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //==============================================================================================================

  Printf("All other processes completed, exiting main process.\n");
}
