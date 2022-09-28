#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "header.h"

char buf[BLKSIZE];

int decFreeInodes(int dev);
int ialloc(int dev);  // allocate an inode number from inode_bitmap
int set_bit(char *buf, int bit);
int tst_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);
int balloc(int dev);
int incFreeInodes(int dev);
int idalloc(int dev, int ino);
int bdalloc(int dev, int bno);
int incFreeBlocks(int dev);
int decFreeBlocks(int dev);