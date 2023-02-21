#include <string.h>
#include "malloc.h"
#include<stdio.h>
void print(char *s)
{
    write(1, s, strlen(s));
}

int main()
{
    char *addr;
    int *x=malloc(4);
	*x=17;
	printf("%d",*x);
    addr = malloc(16);
    free(NULL);
    free((void *)addr + 5);
    if (realloc((void *)addr + 5,10) == NULL)
        print("Hello World\n");
}
