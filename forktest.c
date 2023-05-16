#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>


int main(int argc, char *argv[])
{
	int i =0;
	pid_t pid;
	srand(time(NULL));


	for (i; i<10; i++){
		pid = fork();

		if (pid == 0){
			printf("%d \n", getpid());
			usleep(rand() % 1000000);
			exit(0);
		}
	}


	for (i=0; i<10; i++){
		wait(NULL);
	}
	printf("모든 프로세스 종료");


	return 0;
}

