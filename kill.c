#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main()
{
    int i;
    char buffer[256];
    fgets(buffer, 256, stdin);
    i = atoi(buffer);
    if(kill(i, SIGINT)==-1) perror("kill");
    return 0;
}

