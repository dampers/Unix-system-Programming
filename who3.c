#include <stdio.h>
#include <stdlib.h>
#include <utmp.h>
#include <fcntl.h>
#include <time.h>

void showtime(long);
void show_info(struct utmp *);

extern int utmp_open(char *);
extern int utmp_close();

int main(){
        struct utmp *utbufp, *utmp_next();
        if(utmp_open(UTMP_FILE) == -1){
                perror(UTMP_FILE);
                exit(1);
        }
        while((utbufp = utmp_next()) != ((struct utmp *) NULL))
                show_info(utbufp);
        utmp_close();
        return 0;
}
void show_info(struct utmp *u){
        if(u->ut_type != USER_PROCESS)
                return;
        printf("%-8.8s",u->ut_user);
        printf(" ");
        printf("%-8.8s",u->ut_line);
        printf(" ");
        showtime(u->ut_time);
        printf(" ");
        printf("%-8s",u->ut_host);
        printf("\n");
}
void showtime(long timeval){
        char *cp;
        cp = ctime(&timeval);
        printf("%12.12s",cp+4);
}
