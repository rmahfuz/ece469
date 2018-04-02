1) How to build one-level paging and two-level paging with test cases from ques2: In the one-level/apps/example directory and two-level/apps directory,
make clean; make
make run
make run1
make run2
make run3
make run4
make run5
make run6

How to build heap management: In heap-mgmt/os directory 
make clean; make

In heap-mgmt/apps/heaptest-bestfit directory
make clean; make
make run

2) Nothing unusual about one-level and two-level paging

In the provided heaptest.c file, the expected ouput states that the address of block 2 should be 16480 according to the best fit algorithm. In our implementation of heap management, in this example the heap block starting at address 16480 is merged into the remaining unused free space in the heap. Therefore the best fit algorithm does not find an exact fit at this address and instead allocates at the address 16424. The best fit algorithm should still find a best fit under normal circumstances but because in this case the freed block is next to unused heap space, it is merged and no longer the best fit. So even though there's a small difference in the output of heaptest.c, we believe our implementation method is still correct. 


3) List of external sources referenced:
Lecture notes
