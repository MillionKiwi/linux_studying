#include <linux/kernel.h>
#include <linux/syscalls.h>

#define QUEUE_SIZE(256/sizeof(unit32_t))

static int front = 0, rear =0;
static long queue[QUEUE_SIZE];

SYSCALL_DEFINE1(enqueue,uint32_t,data){
	if (front == ((rear + 1) % QUEUE_SIZE)){
		return -1;
	}
	queue[rear++] = data;
	return 0;
	}
SYSCALL_DEFINE0(dequeue){
	if (front == rear){
		return -1;
	}
	return queue[front--];
}
		
