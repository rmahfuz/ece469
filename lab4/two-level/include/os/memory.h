#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);

//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
// All function prototypes including the malloc and mfree functions go here

int MemoryAllocPage(void);
uint32 MemorySetupPte(uint32 page);
void MemoryFreePage(uint32 page);
uint32* findFreeL2pte();
void MemoryAllocateL2PT(PCB*, uint32);


//for each l2 pagetable, there're two variables. inuse == 0 means not inuse
//inuse == 1 means in use. table[256] contains all the data in this l2 pagetable
typedef struct l2_pagetable{
	int inuse;
	uint32 table[256];

} l2_pagetable;

int malloc();
int mfree();

#endif	// _memory_h_
