#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>


typedef struct{
	char acc_no[6];
	char optype;
	int amount;
} Operation;

int operate(Operation oper, int pid ){
	printf("pid = %d %s %c %d \n", pid, oper.acc_no, oper.optype, oper.amount);
	return 0;
}


int main(int argc, char *argv[])
{
	int i =0;
	pid_t pid;
	char line[100];
	srand(time(NULL));


	for (i; i<10; i++){
		pid = fork();

		if (pid == 0){
			FILE* fp = fopen("operation.dat","r");
			Operation oper;
			while (fgets(line, 100, fp) != NULL){
				sscanf(line, "%s %c %d", oper.acc_no, &oper.optype, &oper.amount);
				operate(oper,getpid());
				usleep(rand() % 1000001);
			}
			exit(0);
		}
	}


	for (i=0; i<10; i++){
		wait(NULL);
	}
	printf("모든 프로세스 종료");


	return 0;
}

