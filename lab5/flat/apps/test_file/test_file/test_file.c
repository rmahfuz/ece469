#include "usertraps.h"
//#include "ostraps.h"
//#include "dlxos.h"
//#include "traps.h"
//#include "disk.h"
//#include "dfs.h"
//#include "files.h"
#include "files_shared.h"
#include "misc.h"

void clear_buf(char* buf) { int i; for(i = 0; i < 100; i++) { buf[i] = '\0';} return;}

void main (int argc, char *argv[])
{
	//Testing for Q6:
	int file_desc, num_bytes;
	char* buf; //char buf[100];
	char* sport1 = "soccer players are messy";
	char* sport2 = "basketball players are generally tall";
	char* sport3 = "rwimming is a winter sport, surprise!";


	file_desc = file_open("sports.txt", "w");
	if (file_desc == FILE_FAIL) { Printf("Failed to open file sports.txt\n"); } else { Printf("Successfully opened sports.txt\n"); }

	num_bytes = file_write(file_desc, sport1, dstrlen(sport1) + 1);
	if (num_bytes != dstrlen(sport1) + 1) { Printf("wrote only %d bytes when trying to write sport1\n", num_bytes); } else {Printf("successfully wrote sport1\n"); }

	num_bytes = file_write(file_desc, sport2, dstrlen(sport2) + 1);
	if (num_bytes != dstrlen(sport2) + 1) { Printf("wrote only %d bytes when trying to write sport2\n", num_bytes); } else {Printf("successfully wrote sport2\n"); }

	if (file_close(file_desc) == FILE_FAIL) { Printf("Failed to close file sports.txt"); }

	file_desc = file_open("sports.txt", "rw");
	if (file_desc == FILE_FAIL) { Printf("Failed to open file sports.txt\n"); } else { Printf("Successfully opened sports.txt\n"); }

	//num_bytes = file_read(file_desc, buf, dstrlen(sport1) + 1);
	//if (num_bytes != dstrlen(sport1) + 1) { Printf("read only %d bytes when trying to read sport1\n", num_bytes); } else { Printf("read sport1 (should be %s): %s\n", sport1, buf); } clear_buf();

	if (file_seek(file_desc, dstrlen(sport1) + 1, FILE_SEEK_SET) == FILE_FAIL) { Printf("seek1 failed\n"); } else { Printf("seek1 succeeded\n"); }

	num_bytes = file_read(file_desc, buf, dstrlen(sport2) + 1);
	if (num_bytes != dstrlen(sport2) + 1) { Printf("read only %d bytes when trying to read sport2\n", num_bytes); } else { Printf("read sport2 (should be %s): %s\n", sport2, buf); } clear_buf(buf);

	Printf("should say that end of file reached:\t");
	num_bytes = file_read(file_desc, buf, dstrlen(sport2) + 1);

	if (file_seek(file_desc, dstrlen(sport2) + 1, FILE_SEEK_END) == FILE_FAIL) { Printf("seek2 failed\n"); } else { Printf("seek2 succeeded\n"); }

	num_bytes = file_write(file_desc, sport3, dstrlen(sport3) + 1);
	if (num_bytes != dstrlen(sport3) + 1) { Printf("wrote only %d bytes when trying to write sport3\n", num_bytes); } else {Printf("successfully wrote sport3\n"); }

	if (file_seek(file_desc, 0, FILE_SEEK_SET) == FILE_FAIL) { Printf("seek3 failed\n"); } else { Printf("seek3 succeeded\n"); }

	num_bytes = file_read(file_desc, buf, dstrlen(sport1) + 1);
	if (num_bytes != dstrlen(sport1) + 1) { Printf("read only %d bytes when trying to read sport1\n", num_bytes); } else { Printf("read sport1 (should be %s): %s\n", sport1, buf); } clear_buf(buf);

	num_bytes = file_read(file_desc, buf, dstrlen(sport3) + 1);
	if (num_bytes != dstrlen(sport3) + 1) { Printf("read only %d bytes when trying to read sport3\n", num_bytes); } else { Printf("read sport3 (should be %s): %s\n", sport3, buf); } clear_buf(buf);

	if (file_close(file_desc) == FILE_FAIL) { Printf("Failed to close file sports.txt"); }

	if (file_delete("sports.txt") == FILE_FAIL) {Printf("Deletion of sports.txt failed\n"); } else {Printf("Deletion of sports.txt succeeded\n"); }

}

