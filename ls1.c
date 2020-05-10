#include <stdio.h>
#include <dirent.h>

void do_ls(char []);
int main(int argc, char *argv[])
{
    if(argc == 1) do_ls(".");
    else
    {
        while(--argc)
        {
            printf("%s:\n", *++argv);
            do_ls(*argv);
        }
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
            printf("%s\n", pdirent->d_name);
        }
        closedir(dir_ptr);
    }

}
