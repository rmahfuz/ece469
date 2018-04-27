#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"
#include "files.h"
#include "files_shared.h"

void clear_buf(char* buf) { int i; for(i = 0; i < 100; i++) { buf[i] = '\0';} return;}
void print_buf(char* buf) { int i; printf("printing buffer: "); for(i = 0; i < 1024; i++) { printf("%c", buf[i]);} printf("\n"); return;}

void RunOSTests() {
  // STUDENT: run any os-level tests here
  //
	int file_handle, i;
	char flower1[] = "rhododendron";
	char flower2[] = "lilly is a nice flower";
	char flower3[] = "daisy is a sweet flower";
	char flower4[] = "rose is a beautiful flower";
	char flower5[] = "lotus is found in muddy water";
	char buffer[100];
	
	/*//Initial testing for Q4
	printf("checking if flowers.txt exists (should not exist) : %d\n", DfsInodeFilenameExists("flowers.txt"));
	file_handle = DfsInodeOpen("flowers.txt");
	//printf("finished opening file\n");
	DfsInodeWriteBytes(file_handle, flower1, 50, dstrlen(flower1) + 1); 
	//printf("finished first write to file\n");
	DfsInodeReadBytes(file_handle, buffer, 50, dstrlen(flower1) + 1);
	printf("read buffer = %s\n", buffer);
	printf("checking if flowers.txt exists (should exist) : %d\n", DfsInodeFilenameExists("flowers.txt"));
	printf("File size of flowers.txt (should be 63): %d\n", DfsInodeFilesize(file_handle));
	DfsInodeDelete(file_handle);
	printf("deleted flowers.txt\n");*/

	 //testing across runs of the simulator for Q4
	if (DfsInodeFilenameExists("flowers.txt") == DFS_FAIL) { //then open the file and write something into it
		file_handle = DfsInodeOpen("flowers.txt");
		DfsInodeWriteBytes(file_handle, flower1, 50, dstrlen(flower1) + 1);  //testing normal write
		DfsInodeWriteBytes(file_handle, flower2, 2044, dstrlen(flower2) + 1);//writing into non-block-aligned set of bytes (virtual blocks 1 and 2)
		DfsInodeWriteBytes(file_handle, flower3, 13315, dstrlen(flower3) + 1);//writing into indirect addressed block (block 13; 1024*13 + 3 = 13315)
		DfsInodeWriteBytes(file_handle, flower4, 15355, dstrlen(flower4) + 1);//writing into indirect addressed block across blocks (blocks 14 and 15; 1024*15 = 15360)
		
		printf("just created and wrote into flowers.txt\n");
	
	}
	else { //just open the file
		file_handle = DfsInodeOpen("flowers.txt");
		printf("flowers.txt already existed\n");
	}
	// reading, to verify successful writes:
	DfsInodeReadBytes(file_handle, buffer, 50, dstrlen(flower1) + 1);
	printf("read buffer should be 'rhododendron': %s\n", buffer);
	DfsInodeReadBytes(file_handle, buffer, 2044, dstrlen(flower2) + 1);
	printf("read buffer should be 'lilly is a nice flower': "); printf("%s\n", buffer);
	DfsInodeReadBytes(file_handle, buffer, 13315, dstrlen(flower3) + 1);
	printf("read buffer should be 'daisy is a sweet flower': %s\n", buffer);
	DfsInodeReadBytes(file_handle, buffer, 15355, dstrlen(flower4) + 1);
	printf("read buffer should be 'rose is a beautiful flower': %s\n", buffer);
	printf("File size of flowers.txt (should be %d): %d\n", 15355 + dstrlen(flower4) + 1, DfsInodeFilesize(file_handle));
	//DfsInodeDelete(file_handle);
	DfsCloseFileSystem(); 
	
	//=================================================================================================================================================================
	//Testing for Q6:
	/*int file_desc, num_bytes;
	char* buf; //char buf[100];
	char* sport1 = "soccer players are messy";
	char* sport2 = "basketball players are generally tall";
	char* sport3 = "swimming is a winter sport, surprise!";


	file_desc = FileOpen("sports.txt", "w");
	if (file_desc == FILE_FAIL) { printf("Failed to open file sports.txt\n"); } else { printf("Successfully opened sports.txt\n"); }

	num_bytes = FileWrite(file_desc, sport1, dstrlen(sport1) + 1);
	if (num_bytes != dstrlen(sport1) + 1) { printf("wrote only %d bytes when trying to write sport1\n", num_bytes); } else {printf("successfully wrote sport1\n"); }

	num_bytes = FileWrite(file_desc, sport2, dstrlen(sport2) + 1);
	if (num_bytes != dstrlen(sport2) + 1) { printf("wrote only %d bytes when trying to write sport2\n", num_bytes); } else {printf("successfully wrote sport2\n"); }

	if (FileClose(file_desc) == FILE_FAIL) { printf("Failed to close file sports.txt"); }*/

	/*file_desc = FileOpen("sports.txt", "rw");
	if (file_desc == FILE_FAIL) { printf("Failed to open file sports.txt\n"); } else { printf("Successfully opened sports.txt\n"); }

	//num_bytes = FileRead(file_desc, buf, dstrlen(sport1) + 1);
	//if (num_bytes != dstrlen(sport1) + 1) { printf("read only %d bytes when trying to read sport1\n", num_bytes); } else { printf("read sport1 (should be %s): %s\n", sport1, buf); } clear_buf();

	if (FileSeek(file_desc, dstrlen(sport1) + 1, FILE_SEEK_SET) == FILE_FAIL) { printf("seek1 failed\n"); } else { printf("seek1 succeeded\n"); }

	num_bytes = FileRead(file_desc, buf, dstrlen(sport2) + 1);
	if (num_bytes != dstrlen(sport2) + 1) { printf("read only %d bytes when trying to read sport2\n", num_bytes); } else { printf("read sport2 (should be %s): %s\n", sport2, buf); } clear_buf(buf);

	printf("should say that end of file reached:\t");
	num_bytes = FileRead(file_desc, buf, dstrlen(sport2) + 1);

	if (FileSeek(file_desc, dstrlen(sport2) + 1, FILE_SEEK_END) == FILE_FAIL) { printf("seek2 failed\n"); } else { printf("seek2 succeeded\n"); }

	num_bytes = FileWrite(file_desc, sport3, dstrlen(sport3) + 1);
	if (num_bytes != dstrlen(sport3) + 1) { printf("wrote only %d bytes when trying to write sport3\n", num_bytes); } else {printf("successfully wrote sport3\n"); }

	if (FileSeek(file_desc, 0, FILE_SEEK_SET) == FILE_FAIL) { printf("seek3 failed\n"); } else { printf("seek3 succeeded\n"); }

	num_bytes = FileRead(file_desc, buf, dstrlen(sport1) + 1);
	if (num_bytes != dstrlen(sport1) + 1) { printf("read only %d bytes when trying to read sport1\n", num_bytes); } else { printf("read sport1 (should be %s): %s\n", sport1, buf); } clear_buf(buf);

	num_bytes = FileRead(file_desc, buf, dstrlen(sport3) + 1);
	if (num_bytes != dstrlen(sport3) + 1) { printf("read only %d bytes when trying to read sport3\n", num_bytes); } else { printf("read sport3 (should be %s): %s\n", sport3, buf); } clear_buf(buf);

	if (FileClose(file_desc) == FILE_FAIL) { printf("Failed to close file sports.txt"); }

	if (FileDelete("sports.txt") == FILE_FAIL) {printf("Deletion of sports.txt failed\n"); } else {printf("Deletion of sports.txt succeeded\n"); }*/

}
