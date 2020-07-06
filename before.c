#include <stdio.h>
#include <unistd.h>

int main()
{
    char *arglist[2];
    arglist[0] = "./after";
    arglist[1] = 0;

    pid_t pid = getpid();
    printf("Before execvp(): %d\n", pid);
    execvp(arglist[0], arglist);
    return 0;
}
