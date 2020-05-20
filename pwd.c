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
    printpathto(get_inode("."));  // 현재 디렉토리의 inode를 구하고, prinntpathto 함수의 인자로 넣는다. 
    putchar('\n');
    return 0;
}

void printpathto(ino_t this_inode)
{
    ino_t my_inode;
    char its_name[BUFSIZ];  // 디렉토리의 이름을 저장할 char 배열 
    if(get_inode("..")!=this_inode) // 상위디렉토리 inode와 현재 디렉토리의 inode를 비교 
    {
        chdir("..");  // 상위 디렉토리로 이동 
        inum_to_name(this_inode, its_name, BUFSIZ);  // this_inode에 저장된 inode의 디렉토리 이름 구하기 
        my_inode = get_inode("."); // 현재 디렉토리의 inode를 구하기 
        printpathto(my_inode); // 재귀 호출 
        printf("/%s", its_name);  // 재귀 호출에서 반환 뒤 디렉토리 이름 출력 
    }
}


void inum_to_name(ino_t inode_to_find, char* namebuf, int buflen)  // inode값 가지고 디렉토리 이름 구하기 
{
    DIR* dir_ptr;   // dir pointer dir_ptr 
    struct dirent* pdirent;
    dir_ptr = opendir(".");  // 현재 디렉토리 호출 
    if(dir_ptr==NULL)
    {
        perror(".");
        exit(1);
    }
    while((pdirent = readdir(dir_ptr))!=NULL) 
    {
        if(pdirent->d_ino == inode_to_find)
        {
            strncpy(namebuf, pdirent->d_name, buflen); //디렉토리의 이름을 char 배열에 복사 
            namebuf[buflen-1] = '\0';
            closedir(dir_ptr);  // 디렉토리 닫기 
            return;
        }
    }
    fprintf(stderr, "error looking for inum %ld\n", inode_to_find);
    exit(1);
}

ino_t get_inode(char *fname)  // 디렉토리 inode구하기 
{
    struct stat info;
    if(stat(fname, &info)==-1)  //stat 함수로 디렉토리 정보 호출 
    {
        fprintf(stderr, "Cannot stat");
        perror(fname);
        exit(1);
    }
    return info.st_ino; // 디렉토리 정보 반환 
}
