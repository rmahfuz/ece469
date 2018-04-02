//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page => 2MB/4KB = 512 pages
static uint32 freemap[MEM_MAX_PAGES / 32]; //num_pages(512) / 32 pages
//static uint32 pagestart;
//static int nfreepages;
static int freemapmax = MEM_MAX_PAGES / 32;


static l2_pagetable l2_pagetables[MEM_MAX_L2_PAGETABLES]; //256
//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryInitModule
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() {
  //0: in use, 1: available
  /*
  int pages;
  int i;
  uint32 mask = 0xFFFFFFFF;

  pages = lastosaddress / MEM_PAGESIZE + ((lastosaddress % MEM_PAGESIZE));
  
  for (i = 0; i < freemapmax; i++){
    freemap[i] = 0;
    if (pages > 32){
      pages -= 32;
    } else {
      freemap[i] = freemap [i] | (mask << (pages % 32));
      pages = 0;
    }
  }*/

  int i;
  int divide;
  int mod;
  uint32 page;
  uint32 mask = 0xFFFFFFFF;

  divide = lastosaddress/MEM_PAGESIZE/32;
  mod = (lastosaddress/MEM_PAGESIZE) % 32;
  for (i = 0; i < freemapmax; i++){
    if (i < divide)
      freemap[i] = 0;
    if (i == divide)
      freemap[i] = 0xFFFFFFFF&(mask << mod);
    if (i > divide)
      freemap[i] = mask;
}

	for (i = 0 ; i < MEM_MAX_L2_PAGETABLES ; i++){
		l2_pagetables[i].inuse = 0;

	}
	
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
  uint32 offset, baseAddr, physical_addr, l1_pageNum, l2_pageNum;

  if(addr > MEM_MAX_VIRTUAL_ADDRESS){
    return MEM_FAIL;
  }

  offset = addr & 0xFFF;
  l1_pageNum = addr >> MEM_L1FIELD_FIRST_BITNUM;
  l2_pageNum = (addr >> MEM_L2FIELD_FIRST_BITNUM) & 0xFF;
  baseAddr = ((uint32*)(pcb->pagetable[l1_pageNum]))[l2_pageNum];
  if (baseAddr & 0x1){ //checking if Page Table Entry is valid, use MEM_PTE_VALID
  physical_addr = ((baseAddr) & 0x1FF000) | offset;
  return physical_addr;
  } else{
    printf("Invalid page table entry for *pcb = %d, addr = %d\n",*pcb, addr);
    //pcb->currentSavedFrame[PROCESS_STACK_FAULT] = addr;
    return MemoryPageFaultHandler(pcb);

  }

}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
	uint32 vpagenum = 0;

  uint32 ppagenum;
	
	uint32 stackpagenum = 0;

  uint32 addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];

  uint32 userPtr = pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER];

	uint32 l2_pageNum = (addr >> MEM_L2FIELD_FIRST_BITNUM) & 0xFF;
	uint32 l1_pageNum = (addr >> MEM_L1FIELD_FIRST_BITNUM);

	vpagenum = addr >> MEM_L2FIELD_FIRST_BITNUM;

  stackpagenum = userPtr >> MEM_L2FIELD_FIRST_BITNUM;


	


  /* // segfault if the faulting address is not part of the stack */
  printf("MemoryPageFaultHandler: vpagenum = %d, stackpagenum = %d\n", vpagenum, stackpagenum);
   if (vpagenum < stackpagenum) { 
     dbprintf('m', "addr = %x\nsp = %x\n", addr, pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]); 
     printf("FATAL ERROR (%d): segmentation fault at page address %x\n", GetPidFromAddress(pcb), addr); 
     ProcessKill(); //argument?
     return MEM_FAIL; 
   } 

   ppagenum = MemoryAllocPage(); 
   //pcb->pagetable[vpagenum] = MemorySetupPte(ppagenum); 
	
	((uint32*)pcb->pagetable[l1_pageNum])[l2_pageNum] = MemorySetupPte(ppagenum);
	
   dbprintf('m', "Returning from page fault handler\n"); 
   return MEM_SUCCESS; 
  //return MEM_FAIL;
}


//---------------------------------------------------------------------
//    printf("After Process fork\n");
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

int MemoryAllocPage(void) {
  int i, j;
  uint32 tmp;

    for(i=0;i < freemapmax; i++){
	//printf("freemap i:%d\n", i);
      if (freemap[i] == 0) continue;
      else{
        tmp = 0x1;
            for(j=0; j <32; j++){
						//printf("freemap j:%d\n", j);
                if((freemap[i] & (tmp << j))){
                    freemap[i] ^= (tmp << j); 
                    return (i*32) + j;
                }
            }
        }

  }
  return MEM_FAIL;
  
}



uint32 MemorySetupPte (uint32 page) {
  return (uint32)((page << 12) | MEM_PTE_VALID);
}


void MemoryFreePage(uint32 page) {  //modifies the freemap
/*
  uint32 page_num = page >> 12;
  int left = page_num % 32;
  freemap[page_num/32] = freemap[page_num/32] ^ (1<<left);
*/
uint32 index = page/32;
uint32 bitnum = page%32;
freemap[index] = (freemap[index]&invert(0x1<<bitnum))|(0x1<<bitnum);
}

int malloc(){
  return 0;
}

int mfree(){
  return 0;
}

//two-level pagetable function for allocate
void MemoryAllocateL2PT(PCB* pcb, uint32 l1_pageNum){
	if ((uint32*)(pcb->pagetable[l1_pageNum]) == NULL) 
		pcb->pagetable[l1_pageNum] = findFreeL2pte();


}

uint32* findFreeL2pte(){
	int i;
	for (i =0; i < MEM_MAX_L2_PAGETABLES ; i++){
		if (l2_pagetables[i].inuse == 0){
			l2_pagetables[i].inuse = 1;
			return (uint32*)(l2_pagetables[i].table);

		}


	}
	return NULL;

}







