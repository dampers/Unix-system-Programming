#include <stdioi.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <string.h>


#define PORTNUM 19166
#define oops(msg) {perror(msg); exit(1);}

extern int make_server_socket(int);
void process_request(int);
void child_waiter(int signum);

int main(int ac, char *av[])
{
    int sock, fd;
    srand((unsigned)clock(NULL));
    signal(SIGCHLD, child_waiter);
    sock = make_server_socket(PORTNUM);
    if(sock == -1)
        exit(1);

    while(1)
    {
        fd = accept(sock, NULL, NULL);
        if(fd == -1) break;

        process_request(fd);
        close(fd);
    }
}

void child_waiter(int signum)
{
    while(waitpid(-1, NULL, WNOHANG)>0);
}

void process_request(int fd)
{
    char id[BUFSIZ];
    char command[BUFSIZ];
    FILE *sock_fpi, *sock_fpo;
    int c;
    FILE *pipe_fp;

    int pid = fork();
    switch(pid)
    {
        case -1:
            return;
        case 0:
            
    }
}
