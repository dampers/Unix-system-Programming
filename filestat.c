#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

void show_stat_info(struct stat * buf);
int main(int argc, char *argv[])
{
    struct stat infobuf;
    if(argc!=2)
    {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(stat(argv[1], &infobuf) == -1)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    show_stat_info(&infobuf);
    return 0;
}

void show_stat_info(struct stat* buf)
{
    printf("File stat:\n");
    printf("    Inode: %ld\n", buf->st_ino);
    printf("    mode: %o\n", buf->st_mode);
    printf("    links: %ld\n", buf->st_nlink);
    printf("    user: %d\n", buf->st_uid);
    printf("    group: %d\n", buf->st_gid);
    printf("    size: %ld\n", buf->st_size);
    printf("    last modification time: %ld\n", buf->st_mtime);

}
