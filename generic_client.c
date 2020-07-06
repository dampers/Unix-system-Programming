#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

extern int connect_to_server(char *, int);
void talk_with_server(int);

#define LEN 256

int main(int ac, char *av[])
{
    int fd;
    fd = connect_to_server(av[1], atoi(av[2]));
    if(fd == -1)
        exit(1);
    talk_with_server(fd);
    close(fd);
}

void talk_with_server(int fd)
{
    char buf[LEN];
    int n;
    n = read(fd, buf, LEN);
    write(1, buf, n);
}
