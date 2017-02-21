#include "blocks.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

int main (void) 
{
    char *filename = "test.di";
    int fd = open(filename,O_CREAT|O_RDWR,0644);
    if (fd<0) return -1;

    arc_header hdr;
    printf("off_t size: %zd\n",sizeof(off_t));
    printf("size of header %zd\n",sizeof(arc_header));
    printf("metdata offset:%jd, size: %jd\n",(intmax_t)hdr.meta_off,(intmax_t)hdr.total);
    header_init(&hdr);
 //   header_write(&hdr,fd);
//    header_load(&hdr,fd);
    printf("metdata offset:%jd, size: %jd\n",(intmax_t)hdr.meta_off,(intmax_t)hdr.total);

    int fd_1 = open("test.file",O_CREAT|O_RDWR,0644);
    off_t far = file_archive(&hdr,fd_1,fd);
//int far = file_extract(&hdr,512,
    printf("far is %jd\n",(intmax_t)far);
    
 int fd_2 = open("test2.file",O_CREAT|O_RDWR,0644);
    far = file_archive(&hdr,fd_2,fd);
//int far = file_extract(&hdr,512,
    printf("far is %jd\n",(intmax_t)far);
    close(fd_2);
    close(fd_1);

    printf("metdata offset:%jd, size: %jd\n",(intmax_t)hdr.meta_off,(intmax_t)hdr.total);
    char *meta = "salamala";
    far = metadata_archive(&hdr,meta,strlen(meta)+1,fd);
    printf("far is %jd\n",(intmax_t)far);
    printf("metdata offset:%jd, size: %jd\n",(intmax_t)hdr.meta_off,(intmax_t)hdr.total);

    header_write(&hdr,fd);
    close(fd);
}


