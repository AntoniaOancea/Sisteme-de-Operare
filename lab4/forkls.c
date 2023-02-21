#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>

int main(){
	pid_t pid = fork();
	if(pid<0){
		perror("Didn't Fork\n");
		return errno;
		}
	else
		if(pid==0){
			printf("My PID = %d, Child PID = %d\n",getppid(),getpid());
			const char* path="/usr/bin/ls";
			char *argv[]={"ls",NULL};
			execve(path,argv,NULL);
			}
		else{
			wait(NULL);
			printf("\nChild %d finished\n",getpid());
			}
	return 0;
}
