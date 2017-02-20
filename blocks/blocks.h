#ifndef BLOCKS_H
#define BLOCKS_H

#include <stddef.h>
#include <unistd.h>

#define BLOCK_SIZE 512

typedef struct arc_header
{
    size_t meta_off;
    size_t total;
} arc_header;

void header_init(arc_header *hdr);
ssize_t header_load(arc_header *hdr, int fd_arc);
ssize_t header_write(arc_header *hdr, int fd_arc);

ssize_t file_archive(arc_header *hdr,int fd_file,int fd_arc);
ssize_t file_extract(int fd_arc,size_t off_arc,size_t file_size,int fd_file);

#endif
