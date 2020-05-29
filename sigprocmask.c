#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main()
{
    int i = 5;
    sigset_t sigs, prevsigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    printf("Critical section in\n");
    sigprocmask(SIG_BLOCK, &sigs, &prevsigs);
    while(i--) sleep(1);
    sigprocmask(SIG_SETMASK, &prevsigs, NULL);
    printf("Critical section out\n");
    while(i--) sleep(1);
}
