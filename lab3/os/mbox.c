#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

static mbox mboxes[MBOX_NUM_MBOXES];		//All mailboxes in system
static mbox_message msgs[MBOX_NUM_BUFFERS];	//All message buffers in system

//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words, 
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

void MboxModuleInit() {
	int i;
	for (i = 0; i < MBOX_NUM_MBOXES; i++){
		mboxes[i].inuse = 0;
	}

	for (i = 0; i < MBOX_NUM_BUFFERS; i++){
		msgs[i].inuse = 0;
	}
}

//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use. 
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {
  mbox_t mbox;
  int intr;
  int i;

  intr = DisableIntrs();
  for(mbox = 0; mbox<MBOX_NUM_MBOXES; mbox++){
    if(mboxes[mbox].inuse == 0){
      mboxes[mbox].inuse = 1;
      mboxes[mbox].lock = LockCreate();
      mboxes[mbox].notFull = CondCreate(mboxes[mbox].lock);
      mboxes[mbox].notEmpty = CondCreate(mboxes[mbox].lock);
      AQueueInit(&(mboxes[mbox].msg_queue));
      for (i = 0; i < PROCESS_MAX_PROCS; i++){
        mboxes[mbox].procs[i] = 0;
      }
      break;
    }
  }

  RestoreIntrs(intr);
  if (mbox == MBOX_NUM_MBOXES) return MBOX_FAIL;



  return mbox;
}

//-------------------------------------------------------
// 
// void MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle 
// of the mailbox and the inuse flag will not be changed 
// during execution.  This allows us to get the a valid 
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxOpen(mbox_t handle) {
  int pid = GetCurrentPid();
  int intr;

 if (LockHandleAcquire(mboxes[handle].lock) != SYNC_SUCCESS) {
    printf("MboxOpen: LockAcquire fail\n");
    return MBOX_FAIL;
  }

  intr = DisableIntrs();
  if (mboxes[handle].inuse == 0) return MBOX_FAIL;

  mboxes[handle].procs[pid] = 1; //indicates that current pid has opened mailbox handle
  RestoreIntrs(intr);

  if (LockHandleRelease(mboxes[handle].lock) != SYNC_SUCCESS){
    printf("MboxOpen: LockRelease fail");
    return MBOX_FAIL;
  }
  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {
  int pid = GetCurrentPid();
  int numprocs = 0;


 if (LockHandleAcquire(mboxes[handle].lock) != SYNC_SUCCESS) {
    printf("MboxClose: LockAcquire fail\n");
    return MBOX_FAIL;
  }
  pid = GetCurrentPid();
  if (mboxes[handle].inuse == 0) return MBOX_FAIL;

  mboxes[handle].procs[pid] = 0;

  for (pid = 0; pid < PROCESS_MAX_PROCS; pid++){
    if (mboxes[handle].procs[pid] == 1)
      numprocs++;
  }

  if (numprocs == 0){
    mboxes[handle].inuse = 0; 
    //reinitialize queue
    AQueueInit(&(mboxes[handle].msg_queue));
  }

  if (LockHandleRelease(mboxes[handle].lock) != SYNC_SUCCESS){
    printf("MboxClose: LockRelease fail");
    return MBOX_FAIL;
  }
    
  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call 
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the 
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) {
  int intr;
  int pid;
  int i;
  Link* lnk;

 if (LockHandleAcquire(mboxes[handle].lock) != SYNC_SUCCESS) {
    printf("MboxSend: LockAcquire fail\n");
    return MBOX_FAIL;
  }

  if(mboxes[handle].inuse == 0) return MBOX_FAIL;
  
  //verify current pid has mailbox open
  pid = GetCurrentPid();
  if(mboxes[handle].procs[pid] == 0) {
    printf("MboxSend: mbox not open");
    return MBOX_FAIL;
  }

  //check message length
  if (length > MBOX_MAX_MESSAGE_LENGTH) {
    printf("MboxSend: message length too long.\n length: %d Max: %d", length, MBOX_MAX_MESSAGE_LENGTH);
    return MBOX_FAIL;
  }
 
  //check if num items in queue is greater than max then wait
  //wait until there is room in queue (not full)
  if (mboxes[handle].msg_queue.nitems >= MBOX_MAX_BUFFERS_PER_MBOX){
    printf("PID(send): %d\n", GetCurrentPid()); 
    CondHandleWait(mboxes[handle].notFull);

    printf("MboxSend: CondWait fail\n");
    return MBOX_FAIL;
  }

  //creating msg struct
  for (i = 0; i < MBOX_NUM_BUFFERS; i++){//find next usable message buffer
    if (msgs[i].inuse == 0){
      msgs[i].inuse = 1;
      break;
    }
  }

  msgs[i].length = length;

  dstrncpy(msgs[i].buffer, (char *)message, (length/sizeof(void*)));
  
  //adding msg struct to mailbox queue
  if ((lnk = AQueueAllocLink(msgs[i].buffer)) == NULL){//allocating in queue
    printf("Could not allocate Link");
    return MBOX_FAIL;
  }
  AQueueInsertLast(&mboxes[handle].msg_queue, lnk);
  

  if (CondHandleSignal(mboxes[handle].notEmpty) != SYNC_SUCCESS) {
    printf("MboxSend: CondSignal fail");
    return MBOX_FAIL;
  }

  if (LockHandleRelease(mboxes[handle].lock) != SYNC_SUCCESS) {
    printf("MboxSend: LockRelease fail");
    return MBOX_FAIL;
  }


  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call 
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".  
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox 
// via MboxOpen.
//   
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
int MboxRecv(mbox_t handle, int maxlength, void* message) {
  int pid;
  Link* l;
  mbox_message *m;

  if (LockHandleAcquire(mboxes[handle].lock) != SYNC_SUCCESS){
    printf("MboxRecv: LockAquire fail");
    return MBOX_FAIL;
  }
  if(mboxes[handle].inuse == 0) return MBOX_FAIL;

  //verify current pid has mailbox open
  pid = GetCurrentPid();
  if(mboxes[handle].procs[pid] == 0) return MBOX_FAIL;

  //wait until message queue has sometihing in it (not empty)
  if (AQueueEmpty(&mboxes[handle].msg_queue)){
    if(CondHandleWait(mboxes[handle].notEmpty) != SYNC_SUCCESS){
      printf("MboxRecv: CondWait fail\n");
      return MBOX_FAIL;
    }
  }

  l = AQueueFirst(&mboxes[handle].msg_queue);
  m = (mbox_message*) l->object;
  
  if (m->length > maxlength){
    printf("Cannot receive message, length too long\nlength: %d max: %d\n",m->length, maxlength * sizeof(void*));
    return MBOX_FAIL;
  }

  dstrncpy(message, (void *)m->buffer, (m->length / sizeof(void *)));
  m->inuse = 0;
  AQueueRemove(&l);

  if (CondHandleSignal(mboxes[handle].notFull) != SYNC_SUCCESS){
    printf("MboxRecv: CondSignal fail");
    return MBOX_FAIL;
  }

  if (LockHandleRelease(mboxes[handle].lock) != SYNC_SUCCESS){
    printf("MboxRecv: LockRelease fail");
    return MBOX_FAIL;
  }
  
  return MBOX_SUCCESS;
}

//--------------------------------------------------------------------------------
// 
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs" list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) {
  int i;
  int intr;

  for (i = 0; i < MBOX_NUM_MBOXES; i++){
    mboxes[i].procs[pid] = 0;
  }
  
  return MBOX_SUCCESS;
}

