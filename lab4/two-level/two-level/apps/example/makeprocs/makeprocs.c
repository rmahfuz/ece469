#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"

void main (int argc, char *argv[])
{
  int part_num = 0;             // Used to store number of processes to create
  int i;                               // Loop index variable
	int num_procs;
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 2) {
    Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  part_num = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  //Printf("makeprocs (%d): Creating %d hello_world processes\n", getpid(), num_hello_world);
  

	switch(part_num) {
		case 0:
		case 1: 
		case 2:
		case 3: 
		case 6: num_procs = 1; break;
		case 4: num_procs = 100; break;
		case 5: num_procs = 30; break;
	}

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(   -(num_procs-1)    ) == SYNC_FAIL)) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  Printf("semaphores created\n");//remove

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  // Create Hello World processes
  Printf("-------------------------------------------------------------------------------------\n");

switch(part_num) {
case 0: {
		Printf("makeprocs (%d): part1: Creating a hello world process\n", getpid());
		process_create(HELLO_WORLD, s_procs_completed_str, NULL);
}
case 1: {
		Printf("makeprocs (%d): part1: Creating a hello world process\n", getpid());
		process_create("part1.dlx.obj", s_procs_completed_str, NULL);
} break;
case 2: {
		Printf("makeprocs (%d): part2: Creating a process to access memory inside the virtual address space, but outside of currently allocated pages\n", getpid());
		process_create("part2.dlx.obj", s_procs_completed_str, NULL);
} break;
case 3: {
		Printf("makeprocs (%d): part3: Creating a process to cause the user function call stack to grow larger than one page\n", getpid());
		process_create("part3.dlx.obj", s_procs_completed_str, NULL);
} break;
case 4: {
		Printf("makeprocs (%d): part4: calling the hello world program 100 times\n", getpid());
		for (i = 0; i < 100; i++) {
			process_create("part4.dlx.obj", s_procs_completed_str, NULL);
		}
} break;
case 5: {
		Printf("makeprocs (%d): part5: spawning 30 simultaneous processes\n", getpid());
		for (i = 0; i < 30; i++) {
			process_create("part5.dlx.obj", s_procs_completed_str, NULL);
		}
} break;
case 6: {
		Printf("makeprocs (%d): part6: Creating a process to access memory beyond maximum virtual address \n", getpid());
		process_create("part6.dlx.obj", s_procs_completed_str, NULL);
} break;

} //end of switch-case

    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }


  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
