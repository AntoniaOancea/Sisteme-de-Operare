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
	printf("Starting parent:%d\n",getpid());
	for(int i=1;i<argc;i++){
		pid_t pid = fork();
		if(pid<0){
			perror("Didn't Fork\n");
			return errno;
			}
		else
			if(pid==0){
				int argument=atoi(argv[i]);
				collatz(argument);
				exit(0);
				}
			
	}
	
	for(int i=1;i<argc;i++){
		wait(NULL);
		printf("\nDone Parent %d Me %d\n",getpid(),getppid());
		}
	return 0;
}
