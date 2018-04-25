#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "files_shared.h"
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
	int i;
	uint32 handle;
	//check if the filename is already opened
	for(i = 0; i < FILE_MAX_OPEN_FILES ; i++){
		if (fdsArray[i].inuse == 0) continue;
		if (dstrncmp(filename, fdsArray[i].filename, FILE_MAX_FILENAME_LENGTH) == 0){
			printf("FileOpen: ERROR: filename %s is already opened in process: %d\n", filename, fdsArray[i].processid);
			return FILE_FAIL;
		}
	}
	//check if there's an inode with same filename exists
	if (DfsInodeFilenameExists(filename) == DFS_FAIL){
		//printf("FileOpen: ERROR: Trying to open file %s, but no corresponding inode found with same filename\n", filename); return FILE_FAIL;
		handle = DfsInodeOpen(filename);
		if (handle == DFS_FAIL) { printf("FileOpen: Could not open inode for filename %s\n", filename); }
		printf("FileOpen: create a new file named %s\n", filename);	
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
				printf("FileOpen: ERROR: unexpected mode %s when trying to open file %s\n", mode, filename);
				return FILE_FAIL;
			}
//processid updated
			//fdsArray[i].processid = getpid();  //please update!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
			fdsArray[i].processid = 30;
//currentposition updated
			fdsArray[i].currentposition = 0;
//eof updated
			fdsArray[i].eof = 0;
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
	printf("FileOpen: ERROR: no available spot in fdsArray when trying to open file %s\n", filename);
	return FILE_FAIL;
}

int FileClose(int file_handle){
	if (file_handle >= 0 && file_handle < FILE_MAX_OPEN_FILES){
		if (fdsArray[file_handle].inuse == 0) {
			printf("FileClose: ERROR: trying to close file handle %d that is not open\n", file_handle);
			return FILE_FAIL;
		}
		if (LockHandleAcquire(lock_descriptor) != SYNC_SUCCESS){ printf("FileClose: ERROR: cannot get lock\n"); return FILE_FAIL; } // acquire lock
		fdsArray[file_handle].inuse = 0;
		if (LockHandleRelease(lock_descriptor) != SYNC_SUCCESS){ printf("FileClose: ERROR: cannot release lock\n"); return FILE_FAIL; } // release lock
		return FILE_SUCCESS;	
	}
	else {
		printf("FileClose: ERROR: trying to close out-of-bounds file handle %d\n", file_handle);
		return FILE_FAIL;
	}
}

int FileRead(int file_handle, void *mem, int num_bytes){
	if (fdsArray[file_handle].inuse != 1){
		printf("FileRead: ERROR: trying to read from a file handle %d that is not open\n", file_handle);
		return FILE_FAIL;
	}
	if (fdsArray[file_handle].mode != 1 && fdsArray[file_handle].mode != 3){
		printf("FileRead: ERROR: File with file handle %d not opened in read/readwrite mode\n", file_handle);
		return FILE_FAIL;
	}
	if (num_bytes > 4096 || num_bytes < 0){
		printf("FileRead: ERROR: number of bytes to read %d is not allowed for file handle %d\n", num_bytes, file_handle);
		return FILE_FAIL;
	}
	if (fdsArray[file_handle].eof != 0){
		printf("FileRead: ERROR:  Cannot read from file with file handle %d because we reached the end of the file\n", file_handle);
		return FILE_FAIL;
	}
//update num_bytes if cur+numbytes > filesize

	if (fdsArray[file_handle].currentposition + num_bytes > 
		DfsInodeFilesize(fdsArray[file_handle].inode)){
		printf("FileRead: WARNING: number of bytes demanded to read %d is more than filesize for file handle %d\n", num_bytes, file_handle);
		num_bytes = DfsInodeFilesize(fdsArray[file_handle].inode) - fdsArray[file_handle].currentposition;
	}	
	if (DfsInodeReadBytes(fdsArray[file_handle].inode, mem, fdsArray[file_handle].currentposition, num_bytes) == DFS_FAIL){  //actually doing the reading
		printf("FileRead: ERROR: cannot read from file %s with file handle %d\n",  fdsArray[file_handle].filename, fdsArray[file_handle].inode);
		return FILE_FAIL;
	}
	fdsArray[file_handle].currentposition += num_bytes;
	if ( fdsArray[file_handle].currentposition ==  DfsInodeFilesize(fdsArray[file_handle].inode) ) {
		fdsArray[file_handle].eof = 1;
	}
	return num_bytes;
}
int FileWrite(int file_handle, void *mem, int num_bytes){
	if (fdsArray[file_handle].inuse != 1){
		printf("FileWrite: ERROR: Trying to write into file with file handle %d, which is not in use\n", file_handle);
		return FILE_FAIL;
	}
	if (fdsArray[file_handle].mode != 2 && fdsArray[file_handle].mode != 3){
		printf("FileWrite: ERROR: Trying to write into file with file handle %d, which is not opened in write/readwrite mode\n", file_handle);
		return FILE_FAIL;
	}
	if (num_bytes > FILE_MAX_READWRITE_BYTES  || num_bytes < 0){
		printf("FileWrite: ERROR: Trying to write %d bytes into file with file handle %d, which is not allowed\n", num_bytes, file_handle);
		return FILE_FAIL;
	}
	/*if (fdsArray[file_handle].eof != 0){
		printf("ERROR: reached the end of the file, cannot write\n");
		return FILE_FAIL;
	}*/
	/*if (fdsArray[file_handle].currentposition + num_bytes > 
		DfsInodeFilesize(fdsArray[file_handle].inode)){
		printf("WARNING: write bytes beyond (FileWrite)filesize\n");
		num_bytes = DfsInodeFilesize(fdsArray[file_handle].inode) 
							- fdsArray[file_handle].currentposition;
	}	*/
	if (DfsInodeWriteBytes(fdsArray[file_handle].inode, mem, 
				fdsArray[file_handle].currentposition, num_bytes) == DFS_FAIL){  //actually do the writing
		printf("FileWrite: ERROR: Could not write into file with file handle %d\n", file_handle);
		return FILE_FAIL;
	}
	fdsArray[file_handle].currentposition = fdsArray[file_handle].currentposition + num_bytes;
	if (fdsArray[file_handle].currentposition >= DfsInodeFilesize(fdsArray[file_handle].inode)){
		fdsArray[file_handle].eof = 1;
	}
	return num_bytes;
}
int FileSeek(int file_handle, int num_bytes, int from_where){
	if (fdsArray[file_handle].inuse != 1){
		printf("FileSeek: ERROR: Trying to seek %d bytes in file with file handle %d, which is unfortunately not open\n", num_bytes, file_handle);
		return FILE_FAIL;
	}
	switch (from_where) {
		case FILE_SEEK_CUR:  {fdsArray[file_handle].currentposition += num_bytes;} break;
		case FILE_SEEK_SET:  {fdsArray[file_handle].currentposition = num_bytes;} break;
		case FILE_SEEK_END:  {fdsArray[file_handle].currentposition = DfsInodeFilesize(fdsArray[file_handle].inode) - num_bytes;} break;
		default: {printf("FileSeek: For file handle %d, invalid from_where: %d\n", file_handle, from_where); return FILE_FAIL;}
	}
	fdsArray[file_handle].eof = 0;  //clearing the eof flag
	return FILE_SUCCESS;
}
int FileDelete(char *filename){
	if (DfsInodeFilenameExists(filename) == DFS_FAIL ) {
		printf("FileDelete: ERROR: Trying to delete file %s that does not exist\n", filename);
		return FILE_FAIL;
	}
	if (DfsInodeDelete(DfsInodeFilenameExists(filename)) == DFS_FAIL){
		printf("FileDelete: ERROR: delete of file %s unsuccessful\n", filename);
		return FILE_FAIL;
	}
	return FILE_SUCCESS;
}
//uint32 FileDescFilenameExists(){return 0;}

