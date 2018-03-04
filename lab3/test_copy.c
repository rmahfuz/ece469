#include "lab2-api.h"

//#define dynamic
#define sleep
/*
  This is the testcase for question 3
  Note: Dynamic priority should be disabled
  Expected output is described below.
*/

typedef struct DB {
  int end;
} DB;

main (int argc, char *argv[])
{
  int number, i, j, offset;
  uint32 handle;
  sem_t spage; sem_t ipc_sem;
  char num_str[10], spage_str[10], handle_str[10], ipc_str[10];
  DB * db;

  switch(argc)
  {
    case 2:  
      handle = shmget();
      db = (DB *)shmat(handle);
      if(db == NULL)
        {
          Printf("Could not map the shared page to virtual address, exiting..\n");
          exit();
        }
      db->end = 0;              // Initially the end flag is 0

      spage = sem_create(0);
      ipc_sem = cond_create(0);
      ditoa(handle, handle_str);
      ditoa(spage, spage_str);

      number = dstrtol(argv[1], NULL, 10);
      Printf("Setting number = %d\n", number);
  
      for(i = 0; i < number; i++)
      {
        //Printf("Current process : %d\n", i);
        ditoa(i, num_str);
        process_create("userprog4.dlx.obj", 1 + i, 0, num_str, 
                       spage_str, handle_str,
                       NULL);     // different p_nice for child process
      }

      sem_wait(spage);            // wait for the children to reach 200
      db->end = 1;                // terminate children processes

      break;
    case 4:
      offset = dstrtol(argv[1], NULL, 10);
      spage = dstrtol(argv[2], NULL, 10);
      //ipc_sem = dstrtol(argv[3], NULL, 10);
      handle = dstrtol(argv[3], NULL, 10);
      db = (DB *)shmat(handle);
      if(db == NULL)
        {
          Printf("Could not map the virtual address to the memory, exiting...\n");
          exit();
        }
        sleep(100);

      for(i = 0; !db->end; i ++)
      {
        for(j = 0; j < 50000; j++)
        {
        //#ifdef sleep
          if(offset == 1 && j % 1000 == 0 && j <= 30000)
            sleep(1);
      
          //# endif

          #ifdef dynamic
            if(offset == 0 && j % 500 == 0 && j <= 3000)
              cond_wait(ipc_sem);

            if(offset == 1 && j % 600 == 0)
              cond_signal(ipc_sem);
          # endif
        }     //waste some time
        Printf("%c%d\n",'A'+offset, i);
        if(i > 200) sem_signal(spage);  //signal end
      }

      Printf("***** Process %d reached %d *****\n", getpid(), i);

      /*
        The expected output for the following run

        dlxsim -x os.dlx.obj -a -u userprog4.dlx.obj 2

        is that the first process reaches an "i" which is roughly half
        of the "i" reached by the second process. This implies the
        second process gets scheduled for twice the time of the first
        process.

        Our output is
        ***** Process 29 reached 203 *****
        ***** Process 30 reached 101 *****
        
        Your output may differ a little bit due to randomness.
      */

      break;
    default:
      Printf("Usage: ");
      Printf(argv[0]);
      Printf(" number\n");
      Printf("argc = %d\n", argc);
      exit();
  }
}