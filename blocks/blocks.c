#include "blocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(x,y) ((x)<(y) ? (x) : (y))

void header_init(arc_header* hdr) 
{
    hdr->meta_off = hdr->total = BLOCK_SIZE;
}

ssize_t header_load(arc_header* hdr,int fd_arc)
{
    lseek(fd_arc,0,SEEK_SET);

    char buffer[BLOCK_SIZE];

    ssize_t n_read;
    if ((n_read=read(fd_arc,buffer,BLOCK_SIZE)) < 0) {
        perror("Reading header from archive\n");
        return -1;
    }

    memcpy(hdr,buffer,sizeof(arc_header));

    return n_read;
}

ssize_t header_write(arc_header *hdr,int fd_arc)
{
    lseek(fd_arc,0,SEEK_SET);

    char buffer[BLOCK_SIZE];
    memset(buffer,0,BLOCK_SIZE);
    memcpy(buffer,hdr,sizeof(arc_header)); 

    ssize_t n_write;
    if ((n_write=write(fd_arc,buffer,BLOCK_SIZE)) != BLOCK_SIZE) {
        perror("Writing header to archive\n"); 
        return -1;
    }

    return n_write;
}

ssize_t file_archive(arc_header *hdr,int fd_file,int fd_arc)
{
    size_t block = hdr->meta_off / BLOCK_SIZE;
    size_t block_off = hdr->meta_off % BLOCK_SIZE;

    lseek(fd_arc,block*BLOCK_SIZE,SEEK_SET);

    char buffer[BLOCK_SIZE];
    ssize_t n_read,n_write,file_read = 0;

    if((n_read = read(fd_arc,buffer,BLOCK_SIZE)) < 0) {
        perror("Reading file\n");
        return -1;
    }

    if((n_read = read(fd_file,buffer+block_off,BLOCK_SIZE-block_off)) < 0) {
        perror("Reading file\n");
        return -1;
    }

    size_t content = block_off + n_read;
    memset(buffer+content,0,BLOCK_SIZE-content);
    lseek(fd_arc,block*BLOCK_SIZE,SEEK_SET);

        printf("outside :buffer :%s\n",buffer);
    if ((n_write = write(fd_arc,buffer,BLOCK_SIZE)) != BLOCK_SIZE) {
        perror("Writing file to archive\n");
        return -1;
    }
    file_read += n_read;

    while((n_read = read(fd_file,buffer,BLOCK_SIZE)) > 0) { 
        printf("inside :buffer :%s\n",buffer);
        memset(buffer+n_read,0,BLOCK_SIZE-n_read);
        if ((n_write = write(fd_arc,buffer,BLOCK_SIZE)) != BLOCK_SIZE) {
            perror("Writing file to archive\n");
            return -1;
        }
        file_read += n_read;
    }

    if (n_read < 0) {
        perror("Reading file\n");
        return -1;
    }

    hdr->meta_off += file_read;

    return file_read;
}

ssize_t file_extract(int fd_arc,size_t off_arc,size_t file_size,int fd_file)
{   
    size_t block = off_arc / BLOCK_SIZE;
    size_t block_off = off_arc % BLOCK_SIZE;

    lseek(fd_arc,block*BLOCK_SIZE,SEEK_SET);

    char buffer[BLOCK_SIZE];
    ssize_t n_read,n_write,file_write = 0;
    size_t true_read;

    if((n_read = read(fd_arc,buffer,BLOCK_SIZE)) < 0) {
        perror("Reading file\n");
        return -1;
    }

    true_read = MIN(file_size,BLOCK_SIZE-block_off);
    if ((n_write = write(fd_file,buffer+block_off,true_read)) != true_read) {
        perror("Writing file to archive\n");
        return -1;
    }
    file_write += n_write;

    while(file_write < file_size && (n_read = read(fd_arc,buffer,BLOCK_SIZE)) > 0) { 
        size_t true_read = MIN(file_size - file_write,n_read);
        if ((n_write = write(fd_file,buffer,true_read)) != true_read) {
            perror("Writing file to archive\n");
            return -1;
        }
        file_write += n_write;
    }
    if (n_read < 0) {
        perror("Reading file\n");
        return -1;
    }

    return file_write;

}
