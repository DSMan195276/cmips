/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_BUF_H
#define INCLUDE_BUF_H

#include <unistd.h>

#define BUF_BLK_UNUSED(blk) ((blk)->allocsize - (blk)->size)

struct buf_blk {
    struct buf_blk *next_blk;

    /* allocsize is total size of block
     * size      is the number (starting from 0) of bytes currently in use
     * offset    is the number of bytes at the beginning not in used
     *
     * (size - offset) is the number of bytes contained in this block
     */
    size_t allocsize, size, offset;
    char buf[];
};

struct buf_fd {
    struct buf_blk *head;
    struct buf_blk *tail;

    int fd;
    unsigned int block_size;
    unsigned int max_empty;
    unsigned short has_line;

    unsigned int errno_ret;
    unsigned int closed_gracefully :1;
};

#define BUF_FILE_OPEN_FLAGS (O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK)
#define BUF_FIFO_OPEN_FLAGS (O_RDWR | O_NONBLOCK)

void buf_init(struct buf_fd *);
void buf_free(struct buf_fd *);
void buf_handle_input(struct buf_fd *);
char *buf_read_line(struct buf_fd *);

#endif
