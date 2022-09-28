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

#include "type.h"
#include "header.h"

char buf[BLKSIZE];

int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int tokenize(char *pathname);
MINODE *iget(int dev, int ino);
void iput(MINODE *mip);
int search(MINODE *mip, char *name);
int getino(char *pathname);
int findmyname(MINODE *parent, u32 myino, char myname[ ]) ;
int traverse(MINODE* source, char *pathname);
int findino(MINODE *mip, u32 *myino);
int get_logical_block(MINODE *mip, int lblock);
int set_logical_block(MINODE *mip, int lblock, int value);
MOUNT *getMountFromDev(int dev);
int initGlobalsForDev();
int my_access(char *filename, int mode);
int my_mip_access(MINODE* mip, int mode);
int isOwner(char *filename);

