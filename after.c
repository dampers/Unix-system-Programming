#include <stdio.h>
#include <unistd.h>

int main()
{
    pid_t pid = getpid();
    printf("After execvp(): %d\n", pid);
    return 0;
}
