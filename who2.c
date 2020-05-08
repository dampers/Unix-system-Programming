#include <stdio.h>
#include <stdlib.h>
#include <utmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

void showtime(long);
void show_info(struct utmp*);

int main()
{
    struct utmp current_record;
    int utmpfd;
    int reclen = sizeof(current_record);

    if((utmpfd = open(UTMP_FILE, O_RDONLY)) == -1)
    {
        perror(UTMP_FILE);
        exit(1);
    }
    while(read(utmpfd, &current_record, reclen)==reclen)
        show_info(&current_record);
    close(utmpfd);
    return 0;
}

void show_info(struct utmp* u)
{
    if(u->ut_type != USER_PROCESS)
        return;
    printf("%s ", u->ut_user);
    printf("%-8.8s ", u->ut_line); //devide name of tty
    showtime(u->ut_time);
    printf("(%-12s)\n", u->ut_host);
}

void showtime(long timeval)
{
   struct tm* ltime = localtime(&timeval);
   printf("%d-%1d%1d-", ltime->tm_year+1900, ltime->tm_mon/10,ltime->tm_mon%10);
   printf("%1d%1d %1d%1d:%1d%1d ", ltime->tm_mday/10, ltime->tm_mday%10, ltime->tm_hour/10, ltime->tm_hour%10, ltime->tm_min/10, ltime->tm_min%10);
}
