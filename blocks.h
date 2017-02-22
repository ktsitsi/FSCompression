#ifndef BLOCKS_H
#define BLOCKS_H

#include <stddef.h>
#include <unistd.h>

#define BLOCK_SIZE 512

typedef struct arc_header
{
    off_t meta_off;
    off_t total;
} arc_header;

void header_init(arc_header *hdr);
ssize_t header_load(arc_header *hdr, int fd_arc);
ssize_t header_write(arc_header *hdr, int fd_arc);

off_t file_archive(arc_header *hdr,int fd_file,int fd_arc);
off_t file_extract(int fd_arc,off_t off_arc,off_t file_size,int fd_file);

off_t metadata_extract(arc_header *hdr,int fd_arc,char *metadata);
off_t metadata_archive(arc_header *hdr,char *metadata,size_t meta_size,int fd_arc);

#endif
