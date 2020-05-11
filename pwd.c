#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

void printpathto(ino_t);
ino_t get_inode(char*);
void inum_to_name(ino_t, char*, int);

int main()
{
    printpathto(get_inode("."));
    putchar('\n');
    return 0;
}

void printpathto(ino_t this_inode)
{
    ino_t my_inode;
    char its_name[BUFSIZ];
    if(get_inode("..")!=this_inode)
    {
        chdir("..");
        inum_to_name(this_inode, its_name, BUFSIZ);
        my_inode = get_inode(".");
        printpathto(my_inode);
        printf("/%s", its_name);
    }
}


void inum_to_name(ino_t inode_to_find, char* namebuf, int buflen)
{
    DIR* dir_ptr;
    struct dirent* pdirent;
    dir_ptr = opendir(".");
    if(dir_ptr==NULL)
    {
        perror(".");
        exit(1);
    }
    while((pdirent = readdir(dir_ptr))!=NULL)
    {
        if(pdirent->d_ino == inode_to_find)
        {
            strncpy(namebuf, pdirent->d_name, buflen);
            namebuf[buflen-1] = '\0';
            closedir(dir_ptr);
            return;
        }
    }
    fprintf(stderr, "error looking for inum %ld\n", inode_to_find);
    exit(1);
}

ino_t get_inode(char *fname)
{
    struct stat info;
    if(stat(fname, &info)==-1)
    {
        fprintf(stderr, "Cannot stat");
        perror(fname);
        exit(1);
    }
    return info.st_ino;
}




