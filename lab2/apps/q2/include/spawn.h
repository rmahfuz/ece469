#ifndef __USERPROG__
#define __USERPROG__

#define BUF_SIZE 12  //size of "Hello World"

typedef struct circ_buf {
	char buf[BUF_SIZE];
	int numprocs;
	volatile int head;
	volatile int tail;
} circ_buf;


#define PRODUCER_FILENAME "producer.dlx.obj"
#define CONSUMER_FILENAME "consumer.dlx.obj"

#endif
