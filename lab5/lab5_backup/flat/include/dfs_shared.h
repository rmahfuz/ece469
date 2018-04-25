#ifndef __DFS_SHARED__
#define __DFS_SHARED__

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  uint32 valid;
  uint32 fsBlocksize; //DFS block size in bytes
  uint32 numFsBlocks; //total number of DFS blocks in file system
  uint32 inodeStartBlock; //Starting block number of array of inodes
  uint32 fbvStartBlock; // starting block number of free block vector
} dfs_superblock;

#define DFS_BLOCKSIZE 1024  // Must be an integer multiple of the disk blocksize
#define DFS_FBV_MAX_NUM_WORDS 512
#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB
#define DFS_NUM_FSBLOCKS 16384
 
#define DFS_FAIL -1
#define DFS_SUCCESS 1
#define DFS_NUM_INODES 192
#define DFS_MAX_FILENAME_SIZE 44
typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 128 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 128 bytes.
  uint32 inuse;
  uint32 fileSize;
  char filename[DFS_MAX_FILENAME_SIZE];//TODO should be 44
  uint32 directAddr[10]; //num of direct addressed blocks
  uint32 indirectAddr;
} dfs_inode;


#endif

