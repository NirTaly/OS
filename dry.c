#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main()
{
// creates a new file having full read/write permissions
    int fd = open("myfile", O_RDWR|O_CREAT, 0666);
    write(fd, "haha\n", 5);
    close(fd); // line 6
    fd = open("myfile", O_RDWR); // line 7
    //write(fd, "test", 4);
    ///*
    close(0);
    close(1);
    dup(fd);
    dup(fd);
    if (fork() == 0)
    {
        char s[100];
        dup(fd);
        scanf("%s ", s);//s is haha
        //printf("\n s is: %s\n",s);
        printf("hello\n");
        write(3, s, strlen(s)); // line 18
        return 0; // line 19
    }
    wait(NULL);
    printf("Father finished\n");
    //*/
    close(fd);
    return 0;
}
