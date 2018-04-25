#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

void clear_buf(char* buf) { int i; for(i = 0; i < 1024; i++) { buf[i] = '\0';} return;}
void print_buf(char* buf) { int i; printf("printing buffer: "); for(i = 0; i < 1024; i++) { printf("%c", buf[i]);} printf("\n"); return;}

void RunOSTests() {
  // STUDENT: run any os-level tests here
	int file_handle, i;
	char flower1[] = "rhododendron";
	char flower2[] = "lilly is a nice flower";
	char flower3[] = "daisy is a sweet flower";
	char flower4[] = "rose is a beautiful flower";
	char flower5[] = "lotus is found in muddy water";
	char buffer[1024];
	
	/*//Initial testing:
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

	//testing across runs of the simulator:
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
	//DfsCloseFileSystem();
}

