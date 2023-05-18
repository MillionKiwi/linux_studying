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

int reclock (int fd, int start, int len, int type){
	struct flock fl;

	switch(type) {
		case F_RDLCK:
		case F_WRLCK:
		case F_UNLCK:
			fl.l_type = type;
			fl.l_whence = SEEK_SET;
			fl.l_start = start;
			fl.l_len = len;
			fcntl (fd, F_SETLKW, &fl);
			return 1;

		default:
			return -1;
	};
}

int pOper(struct Operation oper, int pid){
	
	printf("pid:%d  acc_no:%s ",pid, oper.acc_no);

	if (oper.optype == 'd'){
		char s[] = "deposit";
		printf("%s:%d", s, oper.amount);
	}
	else if (oper.optype == 'w'){
		char s[] = "withdraw";
		printf("%s:%d", s, oper.amount);
	}
	else{
		char s[] = "inquiry";
		printf("%s", s);
	}

	return 0;
}

int pbalance(){
	int i=0;
	int fd;
	struct Account acc;

	fd = open("./account.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);


	for (i; i<5; i++){
		lseek(fd, i*sizeof(struct Account), SEEK_SET);
		read(fd, acc.acc_no, sizeof(acc.acc_no));

		lseek(fd, i*sizeof(acc) + sizeof(acc.acc_no) + sizeof(acc.name), SEEK_SET);
		read(fd, &acc.balance, sizeof(acc.balance));

		printf("acc_no:%s balance:%d \n", acc.acc_no, acc.balance);
	}
		
}

int operate(struct Operation oper){
	int fd;
	int accno;
	int pos;
	int i = 0;	// i+1 번째 계좌
	int e = 0; //계좌 존재 확인
	struct Account acc;

	fd = open("./account.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	
	for	(i; i<5; i++){
		lseek(fd, i*sizeof(struct Account), SEEK_SET);
		read(fd, acc.acc_no, sizeof(acc.acc_no));
		if (!strcmp(acc.acc_no, oper.acc_no)){
			e = 1;
			pos = i*sizeof(struct Account); 
			break;
		}
	}

	if (e == 0){
		pOper(oper,getpid());
		printf(", acc_no %s는 없습니다.\n", oper.acc_no);
		close(fd);
		return 0;
	}
	switch (oper.optype){
		case 'i':	//inquiry
			reclock(fd, pos, sizeof(struct Account), F_RDLCK);
			lseek(fd, pos + sizeof(acc.acc_no) + sizeof(acc.name), SEEK_SET);
			read(fd, &acc.balance, sizeof(acc.balance));
			pOper(oper,getpid());
			printf(" balance:%d\n", acc.balance);
			reclock(fd, pos, sizeof(struct Account), F_UNLCK);
			break;

		case 'w':	//withdraw
			reclock(fd, pos, sizeof(struct Account), F_WRLCK);
			lseek(fd, pos + sizeof(acc.acc_no) + sizeof(acc.name), SEEK_SET);
			read(fd, &acc.balance, sizeof(acc.balance));

			acc.balance -= oper.amount;

			lseek(fd, pos + sizeof(acc.acc_no) + sizeof(acc.name), SEEK_SET);
			write(fd, &acc.balance, sizeof(acc.balance));
			pOper(oper,getpid());
			printf(" balance:%d\n", acc.balance);
			reclock(fd, pos, sizeof(struct Account), F_UNLCK);
			break;

		case 'd':
			reclock(fd, pos, sizeof(struct Account), F_WRLCK);
			lseek(fd, pos + sizeof(acc.acc_no) + sizeof(acc.name), SEEK_SET);
			read(fd, &acc.balance, sizeof(acc.balance));

			acc.balance += oper.amount;

			lseek(fd, pos + sizeof(acc.acc_no) + sizeof(acc.name), SEEK_SET);
			write(fd, &acc.balance, sizeof(acc.balance));
			pOper(oper,getpid());
			printf(" balance:%d\n", acc.balance);
			reclock(fd, pos, sizeof(struct Account), F_UNLCK);
			break;
	};
	close(fd);
	return 0;
}

int main(int argc, char *argv[])
{
	int i =0;
	pid_t pid;
	char line[100];		//buff
	struct Operation oper;
	srand(time(NULL));


	for (i; i<10; i++){
		pid = fork();
	
		if (pid == 0){
			FILE* fp = fopen("operation.dat","r");
			while (fgets(line, 100, fp) != NULL){
				sscanf(line, "%s %c %d", oper.acc_no, &oper.optype, &oper.amount);
				operate(oper);
				usleep(rand() % 1000001);
			}
			fclose(fp);
			exit(0);
		}
	}
	

	for (i=0; i<10; i++){
		wait(NULL);
	}

	pbalance();	

	return 0;
}
