#ifndef __FILES_SHARED__
#define __FILES_SHARED__

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

#define FILE_MAX_FILENAME_LENGTH 44

#define FILE_MAX_READWRITE_BYTES 4096

/*void FileModuleInit();
int FileOpen(char *filename, char *mode);
int FileClose(int file_handle);
int FileRead(int file_handle, void *mem, int num_bytes);
int FileWrite(int file_handle, void *mem, int num_bytes);
int FileSeek(int file_handle, int num_bytes, int from_where);
int FileDelete(char *filename);*/

typedef struct file_descriptor {
  // STUDENT: put file descriptor info here
	int inuse;
	char filename[FILE_MAX_FILENAME_LENGTH];
	int inode;
	int eof;
	int mode;//1->read; 2->write ; 3-> readwrite
	int currentposition;
	int processid;
} file_descriptor;



#define FILE_FAIL -1
#define FILE_EOF -1
#define FILE_SUCCESS 1

#endif
