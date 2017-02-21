#include "blocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

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

off_t file_archive(arc_header *hdr,int fd_file,int fd_arc)
{
    off_t block = hdr->meta_off / BLOCK_SIZE;
    off_t block_off = hdr->meta_off % BLOCK_SIZE;

    lseek(fd_arc,block*BLOCK_SIZE,SEEK_SET);

    char buffer[BLOCK_SIZE];
    ssize_t n_read,n_write;
    off_t file_read = 0;

    if((n_read = read(fd_arc,buffer,BLOCK_SIZE)) < 0) {
        perror("Reading file\n");
        return -1;
    }

    if((n_read = read(fd_file,buffer+block_off,BLOCK_SIZE-block_off)) < 0) {
        perror("Reading file\n");
        return -1;
    }

    //printf("n_read is %d\n",n_read);
    off_t content = block_off + n_read;
    memset(buffer+content,0,BLOCK_SIZE-content);
    lseek(fd_arc,block*BLOCK_SIZE,SEEK_SET);

    if ((n_write = write(fd_arc,buffer,BLOCK_SIZE)) != BLOCK_SIZE) {
        perror("Writing file to archive\n");
        return -1;
    }
    file_read += n_read;

    while((n_read = read(fd_file,buffer,BLOCK_SIZE)) > 0) { 
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

    off_t file_offset = hdr->meta_off;
    hdr->meta_off += file_read;

    return file_offset;
}

off_t file_extract(int fd_arc,off_t off_arc,off_t file_size,int fd_file)
{   
    off_t block = off_arc / BLOCK_SIZE;
    off_t block_off = off_arc % BLOCK_SIZE;

    lseek(fd_arc,block*BLOCK_SIZE,SEEK_SET);

    char buffer[BLOCK_SIZE];
    ssize_t n_read,n_write,true_read;
    off_t file_write = 0;

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
        true_read = MIN(file_size - file_write,n_read);
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

off_t metadata_extract(arc_header *hdr,int fd_arc,char *metadata)
{
    off_t block = hdr->meta_off / BLOCK_SIZE;
    off_t block_off = hdr->meta_off % BLOCK_SIZE;

    lseek(fd_arc,block*BLOCK_SIZE,SEEK_SET);

    char buffer[BLOCK_SIZE];
    ssize_t n_read;
    off_t meta_read = 0;

    if((n_read = read(fd_arc,buffer,BLOCK_SIZE)) < 0) {
        perror("Reading file\n");
        return -1;
    }

    memcpy(metadata,buffer+block_off,BLOCK_SIZE-block_off);
    meta_read += BLOCK_SIZE-block_off;

    while((n_read = read(fd_arc,metadata+meta_read,BLOCK_SIZE)) > 0) {
        meta_read += n_read;
    }

    if (n_read < 0) {
        perror("Reading file\n");
        return -1;
    }

    return meta_read;
}

off_t metadata_archive(arc_header *hdr,char *metadata,size_t meta_size,int fd_arc)
{
    off_t block = hdr->meta_off / BLOCK_SIZE;
    off_t block_off = hdr->meta_off % BLOCK_SIZE;

    lseek(fd_arc,block*BLOCK_SIZE,SEEK_SET);

    char buffer[BLOCK_SIZE];
    ssize_t n_read,n_write;
    off_t meta_cur = 0;

    if((n_read = read(fd_arc,buffer,BLOCK_SIZE)) < 0) {
        perror("Reading file\n");
        return -1;
    }
    
    off_t true_size = MIN(meta_size,BLOCK_SIZE-block_off);
    memcpy(buffer+block_off,metadata,true_size);

    off_t content = block_off + true_size;
    memset(buffer+content,0,BLOCK_SIZE-content);
    lseek(fd_arc,block*BLOCK_SIZE,SEEK_SET);

    if ((n_write = write(fd_arc,buffer,BLOCK_SIZE)) != BLOCK_SIZE) {
        perror("Writing file to archive\n");
        return -1;
    }
    meta_cur += true_size;

    while(meta_cur < meta_size) { 
        true_size = MIN(meta_size-meta_cur,BLOCK_SIZE);
        memcpy(buffer,metadata+meta_cur,true_size);
        memset(buffer+true_size,0,BLOCK_SIZE-true_size);
        if ((n_write = write(fd_arc,metadata+meta_cur,BLOCK_SIZE)) != BLOCK_SIZE) {
            perror("Writing file to archive\n");
            return -1;
        }
        meta_cur += true_size;
    }

    if (n_read < 0) {
        perror("Reading file\n");
        return -1;
    }

    return lseek(fd_arc,0,SEEK_CUR);
}
