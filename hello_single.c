#include <stdio.h>
#include <unistd.h>

#define NUM 5

void print_msg(char *);

int main()
{
    print_msg("Hello");
    print_msg("world\n");
}

void print_msg(char *m)
{
    int i;
    for(i=0;i<NUM;i++)
    {
        printf("%s", m);
        fflush(stdout);
        sleep(1);
    }
}
