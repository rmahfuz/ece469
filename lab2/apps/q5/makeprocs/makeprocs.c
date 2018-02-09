#include "lab2-api.h"
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
  //int i;                          // Loop index variable
  //missile_code *mc;               // Used to get address of shared memory page
  //uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  sem_t h2o_sem, h2_sem, o2_sem, so4_sem, so2_sem, h2so4_sem;
  char h2o_sem_str[10], h2_sem_str[10], o2_sem_str[10], so4_sem_str[10], so2_sem_str[10], h2so4_sem_str[10];
  //char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char num_three_str[10], num_four_str[10], num_five_str[10];

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of H20 molecules> <number of SO4 molecules>\n");
    Exit();
  }

  // command line argument to int
  num_h2o = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  num_so4 = dstrtol(argv[2], NULL, 10); // the "10" means base 10

  // int to command line argument
  ditoa(0.5*num_h2o, num_three_str);
  ditoa(num_so4, num_four_str);
  ditoa(findmin(num_h2o, 0.5*num_h2o + num_so4, num_so4), num_five_str);
  
  // Create semaphore to not exit this process until all other processes are done
  if ((s_procs_completed = sem_create(-3)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(s_procs_completed, s_procs_completed_str);
  //==============================================================================================================
  if ((h2o_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(h2o_sem, h2o_sem_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((h2_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(h2_sem, h2_sem_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((o2_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(o2_sem, o2_sem_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((so4_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(so4_sem, so4_sem_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((so2_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(so2_sem, so2_sem_str);
  //---------------------------------------------------------------------------------------------------------------
  if ((h2so4_sem = sem_create(0)) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  ditoa(h2so4_sem, h2so4_sem_str);
  //==============================================================================================================

  // creating processes
  process_create("one.dlx.obj",   s_procs_completed_str, argv[1], h2o_sem_str, NULL);  // producer of h2o
  process_create("two.dlx.obj",   s_procs_completed_str, argv[2], so4_sem_str, NULL);  // producer of so4
  process_create("three.dlx.obj", s_procs_completed_str, num_three_str, h2o_sem_str, h2_sem_str, o2_sem_str,  NULL);  // consumer of 2h20
  process_create("four.dlx.obj",  s_procs_completed_str, num_four_str, so4_sem_str, so2_sem_str, o2_sem_str,   NULL);  // consumer of so4
  process_create("five.dlx.obj",  s_procs_completed_str, num_five_str, h2_sem_str, o2_sem_str, so2_sem_str, h2so4_sem_str, NULL);  // consumer of h2, o2 and so2

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
