#ifndef __MBOX_OS__
#define __MBOX_OS__

#define MBOX_NUM_MBOXES 16           // Maximum number of mailboxes allowed in the system
#define MBOX_NUM_BUFFERS 50          // Maximum number of message buffers allowed in the system
#define MBOX_MAX_BUFFERS_PER_MBOX 10 // Maximum number of buffer slots available to any given mailbox
#define MBOX_MAX_MESSAGE_LENGTH 50   // Buffer size of 50 for each message

#define MBOX_FAIL -1
#define MBOX_SUCCESS 1

#include "queue.h"
#include "process.h"
#include "synch.h"

//---------------------------------------------
// Define your mailbox structures here
//--------------------------------------------

typedef struct mbox_message {
	char* buffer;//string that holds the message, must verify in mbox.c that it is less than the max length allowed for messages
	int length;	//length of message
	int inuse;	
} mbox_message;

typedef struct mbox {
	Queue msg_queue;	//queue holding messages contained in this mbox
	lock_t lock;		//lock handle for this mbox used in opening, closing, sending, receiving
	cond_t notFull;		//cond handle indicating that msg_queue is not empty
	cond_t notEmpty;	//cond handle indicating that msg_queue is not full
	int procs[PROCESS_MAX_PROCS];	//array where each element represents a pid, 1 indicates that pid has the mailbox open and 0 means closed
									//ex. if procs[20] = 1 that means that the process with pid 20 has opened this mailbox
	int inuse;
} mbox;

typedef int mbox_t; // This is the "type" of mailbox handles

//-------------------------------------------
// Prototypes for Mbox functions you have to write
//-------------------------------------------

void MboxModuleInit();
mbox_t MboxCreate();
int MboxOpen(mbox_t m);
int MboxClose(mbox_t m);
int MboxSend(mbox_t m, int length, void *message);
int MboxRecv(mbox_t m, int maxlength, void *message);
int MboxCloseAllByPid(int pid);

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#endif

