
#define MEM_MAX_VIRTUAL_ADDRESS (1<<22) -1
#include<stdio.h>
int main()
{
	printf("Ours: %d\n", 0x3FFFFF & (~0x3));
	printf("adjunker: %d\n", MEM_MAX_VIRTUAL_ADDRESS - 3);
	return 0;
}
