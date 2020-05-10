#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void do_ls(char []);
void do_stat(char* filename);
void show_file_info(char* filename, struct stat* infobuf);
void mode_to_letter(int mode, char str[]);
char* uid_to_name(uid_t uid);
char* gid_to_name(gid_t gid);

int main(int argc, char* argv[])
{
    if(argc == 1) do_ls(".");
    else
        while(--argc)
        {
            printf("%s:\n", *++argv);
            do_ls(*argv);
        }
}

void do_ls(char dirname[])
{
    DIR* dir_ptr;
    struct dirent* pdirent;
    if((dir_ptr = opendir(dirname)) == NULL)
        fprintf(stderr, "ls: cannot open %s\n", dirname);
    else
    {
        while((pdirent = readdir(dir_ptr))!=NULL)
        {
            do_stat(pdirent->d_name);
        }
        closedir(dir_ptr);
    }
}

void do_stat(char* filename)
{
    struct stat infobuf;
    if(stat(filename, &infobuf) == -1)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    else show_file_info(filename, &infobuf);
}

void show_file_info(char* filename, struct stat* buf)
{
    char modestr[11];
    mode_to_letter(buf->st_mode, modestr);
    printf("%-11s", modestr);
    printf("%ld ", buf->st_nlink);
    printf("%s ", uid_to_name(buf->st_uid));
    printf("%s ", gid_to_name(buf->st_gid));
    printf("%5ld ", buf->st_size);
    printf("%.12s", 4+ctime(&buf->st_mtime));
    printf(" %s\n", filename);
}

void mode_to_letter(int mode, char str[])
{
    strcpy(str, "----------");
    
    if(S_ISDIR(mode)) str[0] = 'd';
    if(S_ISCHR(mode)) str[0] = 'c';
    if(S_ISBLK(mode)) str[0] = 'b';

    if(mode & S_IRUSR) str[1] = 'r';
    if(mode & S_IWUSR) str[2] = 'w';
    if(mode & S_IXUSR) str[3] = 'x';

    if(mode & S_IRGRP) str[4] = 'r';
    if(mode & S_IWGRP) str[5] = 'w';
    if(mode & S_IXGRP) str[6] = 'x';

    if(mode & S_IROTH) str[7] = 'r';
    if(mode & S_IWOTH) str[8] = 'w';
    if(mode & S_IXOTH) str[9] = 'x';

}

char* uid_to_name(uid_t uid)
{
    struct passwd* pw_ptr;
    static char numstr[10];
    if((pw_ptr = getpwuid(uid))==NULL)
    {
        sprintf(numstr, "%d", uid);
        return numstr;
    }
    else return pw_ptr->pw_name;
}

char* gid_to_name(gid_t gid)
{
    struct group* grp_ptr;
    static char numstr[10];
    if((grp_ptr = getgrgid(gid))==NULL)
    {
        sprintf(numstr, "%d", gid);
        return numstr;
    }
    else return grp_ptr->gr_name;

}











