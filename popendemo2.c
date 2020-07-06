#include <stdio.h>
#include <stdlib.h>

void write_data(FILE * stream)
{
    int i;
    for(i=0;i<100;i++)
    {
        fprintf(stream, "%d\n", i);
    }
    if(ferror(stream))
    {
        fprintf(stderr, "Output to stream failed.\n");
        exit(1);
    }
}

int main(void)
{
    FILE* output;
    output = popen("more", "w");
    if(!output)
    {
        fprintf(stderr, "incorrect parameters or too many files.\n");
        exit(1);
    }
    write_data(output);
    if(pclose(output)!=0)
        fprintf(stderr, "Could not run more or other error.\n");
    return 0;
}
