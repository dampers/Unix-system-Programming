#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void f(int);

int main()
{
    int i;
    signal(SIGINT, f);
    for(int i=0;i<5;i++)
    {
        printf("hello \n");
        sleep(1);
    }
//    signal(SIGINT, f);
}

void f(int signum)
{
    printf("OUCH!\n");
}
