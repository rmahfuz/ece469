#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file-level functions here
static file_descriptor fdsArray[FILE_MAX_OPEN_FILES];
static lock_t lock_descriptor;


void FileModuleInit(){
	int i = 0;
	for(i = 0; i < FILE_MAX_OPEN_FILES ; i++){
		fdsArray[i].inuse = 0;
		bzero(fdsArray[i].filename, FILE_MAX_FILENAME_LENGTH);
		fdsArray[i].inode = 0;
		fdsArray[i].eof = 0;
		fdsArray[i].mode = 0;
		fdsArray[i].currentposition = 0;
		fdsArray[i].processid = 0;

	}
	lock_descriptor = LockCreate();

}


uint32 FileOpen(char *filename, char *mode){
	int i = 0 ;
	int handle;
	//check if the filename is already opended
	for(i = 0; i < FILE_MAX_OPEN_FILES ; i++){
		if (fdsArray[i].inuse == 0) continue;
		if (dstrncmp(filename, fdsArray[i].filename, FILE_MAX_FILENAME_LENGTH) == 0){
			printf("ERROR: filename is being opened by processes: %d\n", fdsArray[i].processid);
			return FILE_FAIL;
		}
	}
	//check if there's an inode with same filename exists
	if (DfsInodeFilenameExists(filename) == DFS_FAIL){
		printf("ERROR: no corresponding inode found with same filename\n");
		return FILE_FAIL;
	}
	//lock
	if (LockHandleAcquire(lock_descriptor) != SYNC_SUCCESS){
		printf("ERROR: cannot get lock(FILEOPEN)\n");
		return FILE_FAIL;
	}
	//now create a new descriptor
	for(i = 0 ; i < FILE_MAX_OPEN_FILES ; i++){
		if (fdsArray[i].inuse == 0){
//inuse updated
			fdsArray[i].inuse = 1;
//filename updated
			dstrcpy(fdsArray[i].filename, filename);
//mode updated
			if (dstrncmp("rw", mode, 2) == 0){
				fdsArray[i].mode = 3;
			} else if (dstrncmp("r", mode, 1) == 0){
				fdsArray[i].mode = 1;
			}else if (dstrncmp("w", mode, 1) == 0){
				fdsArray[i].mode = 2;
			}else{
				printf("file open mode cannot be recognized\n");
				return FILE_FAIL;
			}
//processid updated
			fdsArray[i].processid = Getpid();
//inode handle updated
			fdsArray[i].inode = DfsInodeFilenameExists(filename);
			if (LockHandleRelease(lock_descriptor) != SYNC_SUCCESS){
				printf("ERROR: cannot release lock(FILEOPEN)\n");
				return FILE_FAIL;
			}
			return i;
		}
	}
	if (LockHandleRelease(lock_descriptor) != SYNC_SUCCESS){
		printf("ERROR: cannot release lock(FILEOPEN)\n");
		return FILE_FAIL;
	}
	printf("ERROR: fdsArray full, no available spot\n");
	return FILE_FAIL;

}

int FileClose(int file_handle){
	if (file_handle >= 0 && file_handle < FILE_MAX_OPEN_FILES){
		fdsArray[file_handle].inuse = 0;
		return FILE_SUCCESS;	
	}
	return FILE_FAIL;
}

int FileRead(int file_handle, void *mem, int num_bytes){
	if (fdsArray[file_handle].inuse != 1){
		printf("ERROR: file is not inuse(FILEREAD)\n");
		return FILE_FAIL;
	}
	if (fdsArray[file_handle].mode != 1 && fdsArray[file_handle].mode != 3){
		printf("ERROR: the file handle is not in read mode(FILEREAD)\n");
		return FILE_FAIL;
	}
	if (num_bytes > 4096 || num_bytes < 0){
		printf("ERROR: number of bytes to read is not allowed (FileRead)\n");
		return FILE_FAIL;
	}
	if (fdsArray[file_handle].eof != 0){
		printf("ERROR: reached the end of the file, cannot read\n");
		return FILE_FAIL;
	}

//update num_bytes if cur+numbytes > filesize

	if (fdsArray[file_handle].currentposition + num_bytes > 
		DfsInodeFilesize(fdsArray[file_handle].inode)){
		printf("WARNING: read bytes beyond filesize\n");
		num_bytes = DfsInodeFilesize(fdsArray[file_handle].inode) 
							- fdsArray[file_handle].currentposition;
	}	
	if (DfsInodeReadBytes(fdsArray[file_handle].inode, mem, 
				fdsArray[file_handle].currentposition, num_bytes) == DFS_FAIL){
		printf("ERROR: cannot read from inode\n");
		return FILE_FAIL;
	}
	fdsArray[file_handle].currentposition = fdsArray[file_handle].currentposition + num_bytes;
	return num_bytes;



}
int FileWrite(int file_handle, void *mem, int num_bytes){
	if (fdsArray[file_handle].inuse != 1){
		printf("ERROR: file is not inuse(FILEREAD)\n");
		return FILE_FAIL;
	}
	if (fdsArray[file_handle].mode != 2 && fdsArray[file_handle].mode != 3){
		printf("ERROR: the file handle is not in write mode(FILEWRITE)\n");
		return FILE_FAIL;
	}
	if (num_bytes > 4096 || num_bytes < 0){
		printf("ERROR: number of bytes to write is not allowed (FileRead)\n");
		return FILE_FAIL;
	}
	if (fdsArray[file_handle].eof != 0){
		printf("ERROR: reached the end of the file, cannot write\n");
		return FILE_FAIL;
	}
	if (fdsArray[file_handle].currentposition + num_bytes > 
		DfsInodeFilesize(fdsArray[file_handle].inode)){
		printf("WARNING: write bytes beyond filesize\n");
		num_bytes = DfsInodeFilesize(fdsArray[file_handle].inode) 
							- fdsArray[file_handle].currentposition;
	}	

	if (DfsInodeWriteBytes(fdsArray[file_handle].inode, mem, 
				fdsArray[file_handle].currentposition, num_bytes) == DFS_FAIL){
		printf("ERROR: cannot read from inode\n");
		return FILE_FAIL;
	}
	fdsArray[file_handle].currentposition = fdsArray[file_handle].currentposition + num_bytes;
	return num_bytes;


}
int FileSeek(int file_handle, int num_bytes, int from_where){
	
}
int FileDelete(char *filename){
	if (DfsInodeDelete(DfsInodeFilenameExists(filename)) == DFS_FAIL){
		printf("ERROR: delete file unsuccessful\n");
		return FILE_FAIL;
	}
	return FILE_SUCCESS;
}
uint32 FileDescFilenameExists(){return 0;}

