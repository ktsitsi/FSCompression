#include "blocks.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main (void) 
{
    char *filename = "test.di";
    int fd = open(filename,O_CREAT|O_RDWR,0644);
    if (fd<0) return -1;

    arc_header hdr;
    printf("size_t size: %lu\n",sizeof(size_t));
    printf("size of header %lu\n",sizeof(arc_header));
    printf("metdata offset:%zu, size: %zu\n",hdr.meta_off,hdr.total);
    //header_init(&hdr);
//    header_write(&hdr,fd);
    header_load(&hdr,fd);
    printf("metdata offset:%zu, size: %zu\n",hdr.meta_off,hdr.total);

    int fd_1 = open("backup.file",O_CREAT|O_RDWR,0644);
int far = file_extract(fd,512,6875,fd_1);
    //printf("far is %d\n",far);
//    close(fd_1);

    printf("metdata offset:%zu, size: %zu\n",hdr.meta_off,hdr.total);
    header_write(&hdr,fd);
    close(fd);
}


