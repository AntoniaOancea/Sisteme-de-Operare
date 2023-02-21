#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/stat.h>
int main(int argc,char* argv[]){

    int shm_fd;
    char *shm_name = "collatz";

    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if(shm_fd == -1){
        perror(NULL);
        return errno;
    }

    int shm_size = argc * getpagesize();

    int ftrunc = ftruncate(shm_fd, shm_size);
    if(ftrunc == -1)
    {
        perror(NULL);
        shm_unlink(shm_name);
        return errno;
    }


    printf("Starting parent: %d\n", getpid());
    for(int i = 1; i < argc; ++i){
        pid_t pid = fork();
        if(pid < 0){
            perror("Didnt't Fork\n");
            return errno;
        } 
        else if(pid == 0){
            char * shm_ptr = mmap(NULL,getpagesize(), PROT_WRITE, MAP_SHARED, shm_fd, (i - 1) * getpagesize());

            if(shm_ptr == MAP_FAILED){
                perror(NULL);
                shm_unlink(shm_name);
                return errno;
            }


            int argument = atoi(argv[i]);
            shm_ptr += sprintf(shm_ptr, "%d: ", argument);
            shm_ptr += sprintf(shm_ptr,"%d ", argument);
            while(argument > 1){
                if(argument % 2){
                    argument = 3 * argument + 1;
                } else{
                    argument = argument / 2;
                }
                shm_ptr += sprintf(shm_ptr, "%d ", argument);
            }
            shm_ptr += sprintf(shm_ptr,"\n");
            printf("Done child id = %d, parent id = %d\n", getpid(), getppid());
            munmap(shm_ptr, getpagesize());
            exit(0);
            
        } 
    }
    for(int i = 1; i < argc; ++i){
        wait(NULL);
    }
    for(int i = 1; i < argc; ++i){
        char* shm_ptr = mmap(NULL, getpagesize(), PROT_READ, MAP_SHARED, shm_fd, (i-1) * getpagesize());
        if(shm_ptr == MAP_FAILED){
                perror(NULL);
                shm_unlink(shm_name);
                return errno;
            }

        printf("%s", shm_ptr);
        munmap(shm_ptr, getpagesize());
    }
    printf("Done parent id = %d\n", getppid());
    shm_unlink(shm_name);
    return 0;
}
