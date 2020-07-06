#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>


extern int make_server_socket(int);
void process_request(int);
void child_waiter(int signum);

void child_waiter(int signum)
{
    wait(NULL);
}

int main(int ac, char *av[])
{
    int sock, fd;
    signal(SIGCHLD, child_waiter);
    sock = make_server_socket(atoi(av[1]));
    if(sock==-1) exit(1);
    while(1)
    {
        fd = accept(sock, NULL, NULL);
        if(fd==-1) break;
        process_request(fd);
        close(fd);
    }
}

void process_request(int fd)
{
    int pid = fork();
    switch(pid)
    {
        case -1:
            return;
        case 0:
            dup2(fd, 1);
            close(fd);
            execl("/bin/date", "date", NULL);
            perror("execlp");
            exit(1);
    }
}
