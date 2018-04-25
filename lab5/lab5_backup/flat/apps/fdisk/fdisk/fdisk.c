#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_NUM_INODES];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)
int num_filesystem_blocks; 

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk
void FdiskSetFBV(uint32 blockstart);

void main (int argc, char *argv[])
{
	// STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
	// You need to think of the finer details. You can use bzero() to zero out bytes in memory

  //Initializations and argc check //TODO
  int i, j;
  dfs_block tmpblock;
  sb.fsBlocksize = DFS_BLOCKSIZE;
  sb.numFsBlocks = DFS_NUM_FSBLOCKS;
  sb.inodeStartBlock = FDISK_INODE_BLOCK_START;
  sb.fbvStartBlock = FDISK_FBV_BLOCK_START;
  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  sb.valid = 0;

  disksize = DFS_MAX_FILESYSTEM_SIZE; //might need to #include "dfs_shared.h"
  diskblocksize = DFS_BLOCKSIZE;
  num_filesystem_blocks = disksize / diskblocksize;

  // Make sure the disk exists before doing anything else
  if (disk_create() == DISK_FAIL){
    Printf("DISK_FAIL: Disk does not exist\n");
    Exit();
  }
 

  // Write all inodes as not in use and empty (all zeros)
  for (i = 0; i < FDISK_INODE_NUM_BLOCKS/*DFS_INODE_MAX_NUM*/; i++){
    inodes[i].inuse = 0;
    for (j = 0; j < 10; j++){
      inodes[i].directAddr[j] = 0; 
    }
  }

  // Next, setup free block vector (fbv) and write free block vector to the disk
  bzero(tmpblock.data, sb.fsBlocksize);
  /*for (i = sb.fbvStartBlock; i < sb.fbvStartBlock +2; i++){//can we assume always 2. yes
    FdiskWriteBlock(i, &tmpblock);
  }*/
  
  FdiskSetFBV(sb.fbvStartBlock+2);
 
  for (i = sb.fbvStartBlock; i < sb.fbvStartBlock + 2; i++){ // i = 19; i < 21; i++ (i = 19, i = 20)
    bcopy((char *)(fbv + ((i - sb.fbvStartBlock) * 1024)) , tmpblock.data, sb.fsBlocksize); //TODO replace hard coded values (ask TA)
    //bcopy((char *)(fbv + (i - sb.fbvStartBlock) * 1024) , tmpblock.data, sb.fsBlocksize); //TODO replace hard coded values (ask TA)
    //bcopy((fbv + ((i - sb.fbvStartBlock) * 256)) , tmpblock.data, sb.fsBlocksize); //TODO replace hard coded values (ask TA)
    //bcopy((fbv + (i - sb.fbvStartBlock) * 0) , tmpblock.data, sb.fsBlocksize); 
    FdiskWriteBlock(i, &tmpblock);  //writing 1024 bytes
  }
  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  sb.valid = 1;
  bzero(tmpblock.data, sb.fsBlocksize);
  bcopy (&sb,tmpblock.data + (DFS_BLOCKSIZE/2), sizeof(sb));
  FdiskWriteBlock(0, &tmpblock);
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {//takes filesystem blocknum; writes 1024 bytes (1 file system block)
  // STUDENT: put your code here
  if (disk_write_block(blocknum * 2, b->data) == DISK_FAIL){
    Printf("DISK_FAIL: Cannot write block 1 in FdiskWriteBlock\n");
		return DISK_FAIL; Exit();
  }
  if (disk_write_block((blocknum * 2) + 1, (b->data) + (DFS_BLOCKSIZE/2)) == DISK_FAIL){
    Printf("DISK_FAIL: Cannot write block 2 in FDiskWriteBlock\n");
		return DISK_FAIL; Exit();
  }
  Printf("just wrote to disk block %d and %d\n", blocknum*2, blocknum*2 + 1);
	return DFS_BLOCKSIZE;
}

void FdiskSetFBV(uint32 blockstart){//TODO check if this function works how it is supposed to
  int i;
  int element = blockstart / 32;
  int digit   = blockstart % 32;
  uint32 mask = 0xFFFFFFFF;

  for (i = 0; i < DFS_FBV_MAX_NUM_WORDS; i++){
    if (i < element)
      fbv[i] = 0;
    else if (i == element)
     fbv[i] = 0xFFFFFFFF&(mask << digit);
    else if (i > element)
      fbv[i] = mask;
	//Printf("\t%x", fbv[i]);
  }
}

