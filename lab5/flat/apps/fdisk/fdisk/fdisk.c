#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

void main (int argc, char *argv[])
{
	// STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
	// You need to think of the finer details. You can use bzero() to zero out bytes in memory

  //Initializations and argc check //TODO
  int i, j;
  dfs_block tmpblock;
  sb.fsBlocksize = DFS_BLOCKSIZE;
  sb.numFsBlocks = FDISK_INODE_NUM_BLOCKS;
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
  if (DiskCreate() == DISK_FAIL){
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
  bzero(tmpblock.data, sb.fsblocksize);
  for (i = sb.fbvBlockStart; i < sb.fbvBlockStart +2; i++){//can we assume always 2??
    FdiskWriteBlock(i, &tmpblock);
  }
  
  FdiskSetFBV(sb.fbvStartBlock);
 
  for (i = sb.fbvStartBlock; i < sb.fbvStartBlock + 2; i++){
    bcopy(((char *)(fbv + (i - sb.fbvBlockStart) * (sb.fsBlocksize/sizeof(uint32))) , tmpblock.data, sb.fsBlocksize); //TODO replace hard coded values (ask TA)
    FdiskWriteBlock(i, &tmpblock);
  }
  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  sb.valid = 1;
  bzero(tmpblock.data, sb.fsblocksize);
  bcopy (sb,tmpblock.data + (DFS_BLOCKSIZE/2), sizeof(sb));
  FdiskWriteBlock(0, &tmpblock);
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {//takes filesystem blocknum; writes 1024 bytes (1 file system block)
  // STUDENT: put your code here
  if (disk_write_block(blocknum * 2, b->data) == DISK_FAIL){
    Printf("DISK_FAIL: Cannot write block 1 in FdiskWriteBlock\n");
    Exit();
  }
  if (disk_write_block((blocknum * 2) + 1, (b->data) + (DFS_BLOCKSIZE/2)) == DISK_FAIL){
    Printf("DISK_FAIL: Cannot write block 2 in FDiskWriteBlock\n");
    Exit();
  }
}

void FdiskSetFBV(uint32 blockstart){//TODO check if this function works how it is supposed to
  int element = blockstart / 32;
  int digit   = blockstart % 32;
  uint32 mask = 0xFFFFFFFF;

  for (i = 0; i < DFS_MAX_NUM_WORDS; i++){
    if (i < element)
      fbv[i] = 0;
    else if (i == element)
     fbv[i] = 0xFFFFFFFF&(mask << digit);
    else if (i > element)
      fbv[i] = mask;
  }
}
