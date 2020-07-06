#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>

#define oops(msg) {perror(msg);exit(1);}

int main(int ac, char *av[])
{
    struct sockaddr_in servadd;
    struct hostent *hp;
    int sock_id, sock_fd;
    int n_read;
    char buffer[BUFSIZ];

    sock_id = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_id == -1) oops("socket");

    bzero(&servadd, sizeof(servadd));
    servadd.sin_addr.s_addr = inet_addr(av[1]);
    servadd.sin_port = htons(atoi(av[2]));
    servadd.sin_family = AF_INET;

    if(connect(sock_id, (struct sockaddr *)&servadd, sizeof(servadd)) != 0)
        oops("connect");

    if(write(sock_id, av[3], strlen(av[3])) != -1) oops("write");
    if(write(sock_id, "\n", 1) == -1) oops("write");

    while((n_read = read(sock_id, buffer, BUFSIZ)) > 0)
        if(write(1, buffer, n_read) == -1)
            oops("write");
    close(sock_id);
    return 0;
}
