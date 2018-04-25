#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

static dfs_inode inodes[DFS_NUM_INODES ]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; //512 

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

static lock_t lock_fbv;
static lock_t lock_inode;

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------
int chkBlcAllocated(uint32 blocknum){
	
	int divide;
	int remain;
	uint32 mask = 0x1;
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (chkBlcAllocated).\n");
		return DFS_FAIL;
	}
	
	divide = blocknum / 32;
	remain = blocknum % 32;
	if (fbv[divide] & (mask << remain)) return DFS_FAIL;
	else return DFS_SUCCESS;


}
void DfsModuleInit() {
// You essentially set the file system as invalid and then open 
// using DfsOpenFileSystem().
	DfsInvalidate();
	if (lock_fbv = LockCreate() == SYNC_FAIL) { printf("ERROR: Could not create lock for free block vector\n"); }
	if (lock_inode = LockCreate() == SYNC_FAIL) { printf("ERROR: Could not create lock for inode\n"); }
	//printf("lock_fbv = %d\n", lock_fbv); printf("lock_inode = %d\n", lock_inode);
	DfsOpenFileSystem();
}

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
// This is just a one-line function which sets the valid bit of the 
// superblock to 0.
	sb.valid = 0;
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {
//declaration
	disk_block dskB;
	dfs_block dfsB;
	int i;
	

//Basic steps:
// Check that filesystem is not already open
	if(sb.valid){
		printf("ERROR: filesystem is already open.\n");
		return DFS_FAIL;
	}
// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.
	if(DiskReadBlock(1, &dskB) == DISK_FAIL) {
		printf("ERROR: DISK READING FAILURE.\n");
		return DFS_FAIL;
	}
//copy the super block to sb
	bcopy(dskB.data, (char *)&sb, sizeof(sb));



// Copy the data from the block we just read into the superblock in memory
// All other blocks are sized by virtual block size:
// Read inodes
// Read free block vector


	if (sb.valid == 0){
		printf("ERROR: FILESYSTEM IS NOT VALID.\n");
		return DFS_FAIL;
	}

	for (i = sb.inodeStartBlock; i < sb.fbvStartBlock; i++){
		if (DfsReadBlock(i, &dfsB) == DFS_FAIL){
			printf("ERROR: DFS BLOCK INODES READ FAILURE.\n");
			return DFS_FAIL;
		}
		bcopy(dfsB.data, inodes + ((i-sb.inodeStartBlock) * sb.fsBlocksize), sb.fsBlocksize);
	}

	//free block vector

	for (i = sb.fbvStartBlock; i < sb.fbvStartBlock + 2; i++){
		if (DfsReadBlock(i, &dfsB) == DFS_FAIL){
			printf("ERROR: DFS BLOCK FBV READ FAILURE.\n");
			return DFS_FAIL;
		}
	bcopy(dfsB.data, fbv + ((i-sb.fbvStartBlock) * sb.fsBlocksize), sb.fsBlocksize);
	}

// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory
	sb.valid = 0;
	bzero(dskB.data, DISK_BLOCKSIZE);
	bcopy(&sb, dskB.data, sizeof(sb));
	if (DiskWriteBlock(1, &dskB) == DISK_FAIL){
		printf("ERROR: SUPER BLOCK WRITE BACK FAILURE.\n");
		return DFS_FAIL;
	}
	sb.valid = 1;
	return DFS_SUCCESS;


}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
	//declaration
	disk_block dskB;
	dfs_block dfsB;
	int i;

	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsCloseFileSystem).\n");
		return DFS_FAIL;
	}
	//write inodes

	
	for (i = sb.inodeStartBlock; i < sb.fbvStartBlock; i++){
		bcopy(inodes + ((i-sb.inodeStartBlock) * sb.fsBlocksize), dfsB.data, sb.fsBlocksize);
		if (DfsWriteBlock(i, &dfsB) == DFS_FAIL){
			printf("ERROR: DFS BLOCK INODES write FAILURE (DfsCloseFileSystem).\n");
			return DFS_FAIL;
		}
		
	}

	//write fbv
	for (i = sb.fbvStartBlock; i < sb.fbvStartBlock + 2; i++){
		bcopy(fbv + ((i-sb.fbvStartBlock) * sb.fsBlocksize), dfsB.data, sb.fsBlocksize);
		if (DfsWriteBlock(i, &dfsB) == DFS_FAIL){
			printf("ERROR: DFS BLOCK FBV WRITE FAILURE (DfsCloseFileSystem).\n");
			return DFS_FAIL;
		}
	}
	//write sb
	bzero(dskB.data, DISK_BLOCKSIZE);
	bcopy(&sb, dskB.data, sizeof(sb));
	if (DiskWriteBlock(1, &dskB) == DISK_FAIL){
		printf("ERROR: SUPER BLOCK WRITE BACK FAILURE(DfsCloseFileSystem).\n");
		return DFS_FAIL;
	}
	sb.valid = 0;
	return DFS_SUCCESS;

}
//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
int i,j = 0;
uint32 tmp = 0;
uint32 mask = 0x1;
// Check that file system has been validly loaded into memory

	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsAllocateBlock).\n");
		return DFS_FAIL;
	}

// Find the first free block using the free block vector (FBV), mark it in use
	if (LockHandleAcquire(lock_fbv) != SYNC_SUCCESS){
		printf("ERROR: cannot get lock when allocating block(DfsAllocateBlock)\n");
		return DFS_FAIL;
	}

//loop through fbv to find the availabe bit

	for (i = 0 ; i < DFS_FBV_MAX_NUM_WORDS ; i++ ){
		if (fbv[i] == 0) continue;
		tmp = fbv[i];
		//printf("(DfsAllocateBlock): tmp = %u, i = %d\n", tmp, i);
		for (j = 0 ; j < 32 ; j++){
			if (tmp & mask){
				fbv[i] ^= mask;
				if (LockHandleRelease(lock_fbv) != SYNC_SUCCESS){
					printf("ERROR: cannot release lock(DfsAllocateBlock)\n");
					return DFS_FAIL;
				}
				return i*32 + j;
			}
			mask = mask << 1;
		}
	}
	if (LockHandleRelease(lock_fbv) != SYNC_SUCCESS){
		printf("ERROR: cannot release lock(DfsAllocateBlock)\n");
		return DFS_FAIL;
	}
	printf("ERROR: no available block in fbv. \n");
	return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {
// Check that file system has been validly loaded into memory

	int divide;
	int remain; 
	uint32 mask = 0x1;
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsFreeBlock).\n");
		return DFS_FAIL;
	}

	if (LockHandleAcquire(lock_fbv) != SYNC_SUCCESS){
		printf("ERROR: cannot get lock when allocating block(DfsFreeBlock)\n");
		return DFS_FAIL;
	}
	
	divide = blocknum /32;
	remain = blocknum % 32;
	if (divide >= DFS_FBV_MAX_NUM_WORDS){
		printf("ERROR: block num is too large(DfsFreeBlock)\n");
		return DFS_FAIL;

	}
	fbv[divide] = fbv[divide] ^ (mask << remain);

	if (LockHandleRelease(lock_fbv) != SYNC_SUCCESS){
		printf("ERROR: cannot release lock(DfsFreeBlock)\n");
		return DFS_FAIL;
	}
	return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {
	int i;
	disk_block dskB;
	int loopNum = sb.fsBlocksize / DISK_BLOCKSIZE; //2 in our case
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsReadBlock).\n");
		return DFS_FAIL;
	}

	if (chkBlcAllocated(blocknum) == DFS_FAIL){
		printf("ERROR: blocknum is not allocated(DfsReadBlock).\n");
		return DFS_FAIL;

	}

	for(i = 0 ; i < loopNum; i++){
		bzero(dskB.data, DISK_BLOCKSIZE);
		if (DiskReadBlock(blocknum * loopNum + i, &dskB) == DFS_FAIL){
			printf("ERROR: cannot read block from DSK (DfsReadBlock).\n");
			return DFS_FAIL;
		}
		bcopy(dskB.data, b->data + (i * DISK_BLOCKSIZE), DISK_BLOCKSIZE);
	}
	
	return sb.fsBlocksize;


}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){
	int i;
	disk_block dskB;
	int loopNum = sb.fsBlocksize / DISK_BLOCKSIZE; //2 in our case
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsWriteBlock).\n");
		return DFS_FAIL;
	}

	if (chkBlcAllocated(blocknum) == DFS_FAIL){
		printf("ERROR: blocknum is not allocated(DfsWriteBlock).\n");
		return DFS_FAIL;

	}

	for(i = 0 ; i < loopNum; i++){
		bzero(dskB.data, DISK_BLOCKSIZE);
		bcopy(b->data + (i * DISK_BLOCKSIZE), dskB.data, DISK_BLOCKSIZE);
		if (DiskWriteBlock(blocknum * loopNum + i, &dskB) == DFS_FAIL){
			printf("ERROR: cannot write block from DSK (DfsWriteBlock).\n");
			return DFS_FAIL;
		}
		
	}
	
	return sb.fsBlocksize;
}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

uint32 DfsInodeFilenameExists(char *filename) {
	int i;
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsInodeFilenameExists).\n");
		return DFS_FAIL;
	}
	for(i=0; i < DFS_NUM_INODES ; i++){
		if (inodes[i].inuse == 1){
			if (dstrncmp(filename, inodes[i].filename, DFS_MAX_FILENAME_SIZE) == 0) return i;
		}
	}
	return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

uint32 DfsInodeOpen(char *filename) {
	uint32 tmpHandle;
	int i, j;
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsInodeOpen).\n");
		return DFS_FAIL;
	}
	tmpHandle = DfsInodeFilenameExists(filename);
	if (tmpHandle != DFS_FAIL) return tmpHandle;
	
	//create a new inode
	//printf("LockHandleRelease(lock_inode) == SYNC_SUCCESS: %d\n", LockHandleRelease(lock_inode) == SYNC_SUCCESS);
	//printf("going to acquire lock to open inode array (DfsInodeOpen)\n");
	//printf("(LockHandleAcquire(lock_inode == SYNC_FAIL) : %d \n", (LockHandleAcquire(lock_inode) == SYNC_FAIL));
	//while (LockHandleAcquire(lock_inode) != SYNC_SUCCESS);
	if (LockHandleAcquire(lock_inode) != SYNC_SUCCESS){
		printf("ERROR: cannot get lock when opening inode array(DfsInodeOpen)\n");
		return DFS_FAIL;
	}
	
	for (i = 0 ; i < DFS_NUM_INODES ; i++){
		if (inodes[i].inuse == 1) continue;
		inodes[i].inuse = 1;
		inodes[i].fileSize = 0;
		dstrncpy(inodes[i].filename, filename, DFS_MAX_FILENAME_SIZE);
		for(j = 0; j < 10 ; j++){
			inodes[i].directAddr[j] = 0;
		}  
		inodes[i].indirectAddr = 0; 
		break;
	}
	
	if (LockHandleRelease(lock_inode) != SYNC_SUCCESS){
		printf("ERROR: cannot release lock(DfsInodeOpen)\n");
		return DFS_FAIL;
	}

	if (i == DFS_NUM_INODES){
		printf("ERROR: cannot create a new inode because the array is full\n");
		return DFS_FAIL;
	}
	return i;

}
//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------
int DfsInodeDelete(uint32 handle) {
	int i;
	dfs_block dfsB;
	uint32 tmpTable[sb.fsBlocksize/sizeof(uint32)];//in our case 256
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsInodeDelete).\n");
		return DFS_FAIL;
	}
	if(inodes[handle].inuse == 0){
		printf("ERROR: inode is not inuse(DfsInodeDelete).\n");
		return DFS_FAIL;
	}
	inodes[handle].fileSize = 0;//filesize set to zero
	bzero(inodes[handle].filename, DFS_MAX_FILENAME_SIZE);//filename set to zero
//free direct array
	for(i = 0; i < 10 ; i++){
		if (inodes[handle].directAddr[i] != 0){
			if (DfsFreeBlock(inodes[handle].directAddr[i]) == DFS_FAIL){
				printf("ERROR: cannot free element in direct table (DfsInodeDelete).\n");
				return DFS_FAIL;
			}
			inodes[handle].directAddr[i] = 0;
		}
		//break;
	}
//free contents in indirectAddr
	if (inodes[handle].indirectAddr != 0) {
		if (DfsReadBlock(inodes[handle].indirectAddr, &dfsB) == DFS_FAIL){
			printf("ERROR: cannot read inode (DfsInodeDelete).\n");
			return DFS_FAIL;
		}		
		bcopy(dfsB.data, tmpTable, sb.fsBlocksize);
		//for (i = 0; i < sizeof(tmpTable); i++){
		for (i = 0; i < sb.fsBlocksize/sizeof(uint32); i++){
			if (tmpTable[i] == 0) continue;
			if (DfsFreeBlock(tmpTable[i]) == DFS_FAIL){
				printf("ERROR: cannot free element in indirect table (DfsInodeDelete).\n");
				return DFS_FAIL;
			}
		}
		//free indirectAddr itself
		if (DfsFreeBlock(inodes[handle].indirectAddr) == DFS_FAIL){
			printf("ERROR: cannot free indirectAddr itself (DfsInodeDelete).\n");
			return DFS_FAIL;
		}
	}
	inodes[handle].indirectAddr = 0;
	if (LockHandleAcquire(lock_inode) != SYNC_SUCCESS){
		printf("ERROR: cannot get lock when deleting inode(DfsInodeDelete)\n");
		return DFS_FAIL;
	}
	inodes[handle].inuse = 0;
	if (LockHandleRelease(lock_inode) != SYNC_SUCCESS){
		printf("ERROR: cannot release lock when deleting inode(DfsInodeDelete)\n");
		return DFS_FAIL;
	}
	//write inodes[handle] back
	return DFS_SUCCESS;
}
//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
	int  j;
	int tmpByte, writeByte;
	int startAt, endAt;
	uint32 tmpAddr;
	dfs_block dfsB;

	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsInodeDelete).\n");
		return DFS_FAIL;
	}
	
	if(inodes[handle].inuse == 0){
		printf("ERROR: inode is not inuse(DfsInodeDelete).\n");
		return DFS_FAIL;
	}
	
	tmpByte = start_byte;
	writeByte = 0;

	while (tmpByte<start_byte + num_bytes) {
		//printf("DfsInodeReadBytes: handle = %d, tmpByte/sb.fsBlocksize = %d\n", handle, tmpByte/sb.fsBlocksize);
		tmpAddr = DfsInodeTranslateVirtualToFilesys(handle, tmpByte/sb.fsBlocksize);
		if ( tmpAddr== DFS_FAIL){
			printf("ERROR: cannot translate virtual block to filesys block (DfsInodeReadBytes).\n");
			return DFS_FAIL;
		}
		//printf("(DfsInodeReadBytes): tmpAddr = %d\n", tmpAddr);
		//printf("here2\n");
		if (DfsReadBlock(tmpAddr, &dfsB) == DFS_FAIL){
			printf("ERROR: cannot read file sys block (DfsInodeReadBytes).\n");
			return DFS_FAIL;
		}
		//printf("here3\n");
		startAt = 0; endAt = sb.fsBlocksize;
		if (start_byte == tmpByte) { startAt = tmpByte % sb.fsBlocksize; } //first time
		if (tmpByte + (endAt - startAt) > start_byte + num_bytes) { endAt = ((start_byte + num_bytes) % sb.fsBlocksize); } //last time
		//if (tmpByte + sb.fsBlocksize > start_byte + num_bytes) { endAt = ((start_byte + num_bytes) % sb.fsBlocksize); } //last time
		//printf("startAt = %d, endAt = %d\n", startAt, endAt);
		for (j = startAt; j < endAt; j++) {
			//printf("here4\n");
			*((char*)(mem + writeByte)) = dfsB.data[j];
			//printf("%c", *((char*)(mem + writeByte)));
			tmpByte++; writeByte++;
		}	
		//printf("\n");
	}
	return writeByte;
}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------

int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
	//Initializations
	int i;
	int tmpByte, readByte;
	int startAt, len_to_write; // number of bytes to write
	uint32 tmpAddr; // virtual block number to write into
	dfs_block dfsB; // place to store the block to write

	//Check if file system is valid
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsInodeWriteBytes).\n");
		return DFS_FAIL;
	}
	//Check if handle is valid
	if(inodes[handle].inuse == 0){
		printf("ERROR: inode is not inuse(DfsInodeWriteBytes).\n");
		return DFS_FAIL;
	}
	//Actually do the writing
	readByte = 0; tmpByte = start_byte;
	while (readByte < num_bytes) {
		// calculate startAt and len_to_write
		startAt = 0; len_to_write = sb.fsBlocksize;
		if (tmpByte == start_byte) {   //first time
			startAt = tmpByte % sb.fsBlocksize;
			len_to_write = sb.fsBlocksize - startAt; //printf("this\n");
		}
		//if (tmpByte + sb.fsBlocksize > start_byte + num_bytes) {   //last time
		if (tmpByte + len_to_write > start_byte + num_bytes) {   //last time
			len_to_write = ((start_byte + num_bytes) % sb.fsBlocksize) - startAt; //printf("this too \n");
		}
		//printf("startAt = %d, len_to_write = %d\n", startAt, len_to_write);
		//len_to_write = sb.fsBlocksize - (tmpByte % sb.fsBlocksize);
		bcopy(mem + readByte, dfsB.data + startAt, len_to_write);
		//bcopy(mem + readByte, dfsB.data + (tmpByte % sb.fsBlocksize), len_to_write);
		// allocate a virtual block
		//printf("DfsInodeWriteBytes: tmpByte = %d, tmpByte/sb.fsBlocksize = %d\n", tmpByte, tmpByte/sb.fsBlocksize);
		tmpAddr = DfsInodeAllocateVirtualBlock(handle, tmpByte/sb.fsBlocksize);
		//if (tmpByte/sb.fsBlocksize >= 10)printf("(DfsInodeWriteBytes): virtual_blocknum = %d, tmpAddr = %d\n", tmpByte/sb.fsBlocksize, tmpAddr);
		if ( tmpAddr== DFS_FAIL){
			printf("ERROR: cannot allocate virtual block(DfsInodeWriteBytes)\n");
			return DFS_FAIL;
		}
		if (DfsWriteBlock(tmpAddr, &dfsB) == DFS_FAIL) {  // actual writing
			printf("ERROR: Cannot write block into disk (DfsInodeWriteBytes)\n");
			return DFS_FAIL;
		}
		tmpByte += len_to_write; readByte += len_to_write;
	}
	//update fileSize
	if (inodes[handle].fileSize < start_byte + readByte)
		inodes[handle].fileSize = start_byte + readByte;
	return readByte;
}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeFilesize(uint32 handle) {
	//Check if file system is valid
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsInodeFilesize).\n");
		return DFS_FAIL;
	}
	//Check if handle is valid
	if(inodes[handle].inuse == 0){
		printf("ERROR: inode is not inuse(DfsInodeFilesize).\n");
		return DFS_FAIL;
	}
	return inodes[handle].fileSize;
}
//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------
uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {
	int i;
	dfs_block dfsB;
	uint32 tmpTable[sb.fsBlocksize/sizeof(uint32)];//in our case 256
	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsInodeAllocateVirtualBlock).\n");
		return DFS_FAIL;
	}
	if(inodes[handle].inuse == 0){
		printf("ERROR: inode is not inuse(DfsInodeAllocateVirtualBlock).\n");
		return DFS_FAIL;
	}
	if (virtual_blocknum < 0 || virtual_blocknum > sb.fsBlocksize/sizeof(uint32) + 10){
		printf("ERROR: virtual_blocknum out of bound(DfsInodeAllocateVirtualBlock).\n");
		return DFS_FAIL;
	}
	if (virtual_blocknum < 10){
		if (inodes[handle].directAddr[virtual_blocknum] == 0){
//this direct addr has never been used, need allocate new block
			inodes[handle].directAddr[virtual_blocknum] = DfsAllocateBlock();
			//printf("DfsAllocateBlock() returned this: %d\n", inodes[handle].directAddr[virtual_blocknum]);
			if (inodes[handle].directAddr[virtual_blocknum] == DFS_FAIL){
				printf("ERROR: directAddr cannot be allocated(DfsInodeAllocateVirtualBlock).\n");
				return DFS_FAIL;	
			}
			//printf("DfsInodeAllocateVirtualBlock: allocated inodes[%d].directAddr[%d] = %d\n", handle, virtual_blocknum, inodes[handle].directAddr[virtual_blocknum]);
			return inodes[handle].directAddr[virtual_blocknum];
		}
		else{
//this direct addr already points to a corresponding file sys block
//CHECK WITH TA
			//printf("DfsInodeAllocateVirtualBlock: already allocated inodes[%d].directAddr[%d] = %d\n", handle, virtual_blocknum, inodes[handle].directAddr[virtual_blocknum]);
			return inodes[handle].directAddr[virtual_blocknum];
		}
	}
	else{
//virtual_blocknum larger than 10. Access indirect address
		if (inodes[handle].indirectAddr == 0){
//indirect address never been initialized, need to allocate indirect Addr first
			inodes[handle].indirectAddr = DfsAllocateBlock();
			if (inodes[handle].indirectAddr == DFS_FAIL){
				printf("ERROR: indirectAddr cannot be allocated(DfsInodeAllocateVirtualBlock).\n");
				return DFS_FAIL;	
			}
//initialize the contents of file sys block pointed by indirectAddr to zero
			bzero(dfsB.data, sb.fsBlocksize);
			if (DfsWriteBlock(inodes[handle].indirectAddr, &dfsB) == DFS_FAIL){
				printf("ERROR: cannot write inodes (DfsInodeTranslateVirtualToFilesys).\n");
				return DFS_FAIL;
			}		
		}
//now indirectAddr should be available, now allocating
		bzero(dfsB.data, sb.fsBlocksize);
		if (DfsReadBlock(inodes[handle].indirectAddr, &dfsB) == DFS_FAIL){
			printf("ERROR: cannot read inode (DfsInodeTranslateVirtualToFilesys).\n");
			return DFS_FAIL;
		}		
		bcopy(dfsB.data, tmpTable, sb.fsBlocksize);

//CHECK WITH TA		
		if (tmpTable[virtual_blocknum-10] == 0){
			tmpTable[virtual_blocknum-10] = DfsAllocateBlock();
			if (tmpTable[virtual_blocknum-10] == DFS_FAIL){
				printf("ERROR: indirectAddr ele cannot be allocated(DfsInodeAllocateVirtualBlock).\n");
				return DFS_FAIL;
			}
		}

		bcopy(tmpTable, dfsB.data, sb.fsBlocksize);
		if (DfsWriteBlock(inodes[handle].indirectAddr, &dfsB) == DFS_FAIL){
			printf("ERROR: cannot read inode (DfsInodeTranslateVirtualToFilesys).\n");
			return DFS_FAIL;
		}
		return tmpTable[virtual_blocknum - 10];
	}
}
//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {
	int i;
	dfs_block dfsB;
	uint32 tmpTable[sb.fsBlocksize/sizeof(uint32)];//in our case 256

	if(sb.valid == 0){
		printf("ERROR: filesystem is not open (DfsInodeTranslateVirtualToFilesys).\n");
		return DFS_FAIL;
	}
	
	if(inodes[handle].inuse == 0){
		printf("ERROR: inode is not inuse(DfsInodeTranslateVirtualToFilesys).\n");
		return DFS_FAIL;
	}

	if (virtual_blocknum < 10){
//direct table address
		return inodes[handle].directAddr[virtual_blocknum];
	}else{
//indirect table address
		if (inodes[handle].indirectAddr == 0){
			printf("ERROR: trying to access block %d in file %d in indirectAddr but indirectAddr is zero(DfsInodeTranslateVirtualToFilesys).\n",virtual_blocknum, handle);
			return DFS_FAIL;
		}
		if (DfsReadBlock(inodes[handle].indirectAddr, &dfsB) == DFS_FAIL){
			printf("ERROR: cannot read inode (DfsInodeTranslateVirtualToFilesys).\n");
			return DFS_FAIL;
		}		
		bcopy(dfsB.data, tmpTable, sb.fsBlocksize);

		if (virtual_blocknum >= (sb.fsBlocksize/sizeof(uint32)) + 10){
			printf("ERROR: virtual_blocknum is too big (DfsInodeTranslateVirtualToFilesys).\n");
			return DFS_FAIL;
		}

		return tmpTable[virtual_blocknum - 10];

	}

}

