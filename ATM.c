#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

struct Operation{
	char acc_no[6];
	char optype;
	int amount;
};

struct Account{
	char acc_no[6];
	char name[10];
	int balance;
};

int reclock (int fd, int accno, int len, int type){
	struct flock fl;

	switch(type) {
		case F_RDLCK:
		case F_WRLCK:
		case F_UNLCK:
			fl.l_type = type;
			fl.l_whence = SEEK_SET;
			fl.l_start = accno * len;
			fl.l_len = len;
			fcntl (fd, F_SETLKW, &fl);
			return 1;

		default:	return -1;
	}
}

int operate(struct Operation oper, int pid ){
	int fd;
	int accno;
	int i = 0;
	struct Account acc;
	int e = 0; //계좌 존재 확인

	fd = open("./account.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	
	for	(i; i<5; i++){
		lseek(fd, i*sizeof(acc), SEEK_SET);
		read(fd, acc.acc_no, sizeof(acc.acc_no));
		if (!strcmp(acc.acc_no, oper.acc_no)){
			e = 1;
			break;
		}
	}

	if (e == 0){
		printf("%d %s %c %d, acc_no %s는 없습니다.\n", pid, oper.acc_no, oper.optype, oper.amount, oper.acc_no);
		close(fd);
		return 0;
	}
	
	printf("%d %s %c %d \n", pid, oper.acc_no, oper.optype, oper.amount);


	close(fd);
	return 0;
}

int pbalance(){
	int i=0;
	int fd;
	struct Account acc;

	fd = open("./account.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);


	for (i; i<5; i++){
		lseek(fd, i*sizeof(acc), SEEK_SET);
		read(fd, acc.acc_no, sizeof(acc.acc_no));

		lseek(fd, sizeof(acc.acc_no) + sizeof(acc.name), SEEK_CUR);
		read(fd, acc.balance, sizeof(acc.balance));

		printf("acc_no:%s balance:%d \n", acc.acc_no, acc.balance);
	}
		
}

int main(int argc, char *argv[])
{
	int i =0;
	pid_t pid;
	char line[100];		//buff
	struct Operation oper;


	for (i; i<10; i++){
		pid = fork();

		if (pid == 0){
			FILE* fp = fopen("operation.dat","r");
			while (fgets(line, 100, fp) != NULL){
				sscanf(line, "%s %c %d", oper.acc_no, &oper.optype, &oper.amount);
				operate(oper,getpid());
			}
			fclose(fp);
			exit(0);
		}
	}


	for (i=0; i<10; i++){
		wait(NULL);
	}
	


	return 0;
}
