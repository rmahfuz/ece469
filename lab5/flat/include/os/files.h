#ifndef __FILES_H__
#define __FILES_H__

#include "dfs.h"
#include "files_shared.h"
/*
#define FILE_MODE_R 0x1;
#define FILE_MODE_W 0x2;
#define FILE_MODE_RW 0x3;

*/
void FileModuleInit();
uint32 FileOpen();
int FileClose();
int FileRead();
int FileWrite();
int FileSeek();
int FileDelete();
uint32 FileDescFilenameExists();


//#define FILE_MAX_OPEN_FILES 15



#endif
