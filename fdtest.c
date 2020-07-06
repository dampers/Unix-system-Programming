#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

char buf[4096];
char problem_buf[33][1024];
char correct_buf[33][1024];

int main()
{
    int fd = open("problem.txt", O_RDONLY);
    if(fd==-1)
    {
        perror("ERROR");
    }
    int n;
    n = read(fd, buf, 4096);
    //printf("%s\n", buf);
    int cnt = 0, i, len = strlen(buf);
    //printf("%d\n", len);
    //for(i=0;i<len;i++)
    //{
    //    if(buf[i]=='\n') cnt++;
    //}
    //printf("cnt = %d\n", cnt);
    //printf("%c\n", buf[1738]);
    return 0;
}
