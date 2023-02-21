#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
void collatz(int argument){
	printf("%d : %d ",argument,argument);
	while(argument>1){
		if(argument % 2) argument=3*argument+1;
		else
			argument=argument/2;
		printf("%d ",argument);
		}
	printf("\n");
}
int main(int argc,char* argv[]){
	if(argc<2){
		perror("Few arguments\n");
		return errno;
	}
	pid_t pid = fork();
	if(pid<0){
		perror("Didn't Fork\n");
		return errno;
		}
	else
		if(pid==0){
			printf("My PID = %d, Child PID = %d\n",getppid(),getpid());
			int argument=atoi(argv[1]);
			collatz(argument);
			}
		else{
			wait(NULL);
			printf("\nChild %d finished\n",pid);
			}
	return 0;
}
