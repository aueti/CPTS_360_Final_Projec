#include "open_close.h"

int my_open_file(char *filePath, int mode)
{
    int ino = getino(filePath);
    MINODE *mip = iget(dev, ino);

    if ((mip->INODE.i_mode & 0xF000) != 0x8000){
        printf("Cannot open: not a file.\n");
        return -1;
    }

    for (int i = 0; i < NFD; i++){
        if (running->fd[i] != 0 && running->fd[i]->minodePtr == mip && running->fd[i]->mode != 0){
            printf("File is currently open in an incompatible mode.\n");
            return -1;
        }
    }

    //Find unused oft & initialize
    OFT *freeOft = oftTable;
    while(freeOft->refCount != 0){
        freeOft++;
    }

    freeOft->mode = mode;
    freeOft->refCount = 1;
    freeOft->minodePtr = mip;

    switch(mode){
        case 0:
            freeOft->offset = 0;
            break;
        case 1:
            if (DEBUG) printf("Truncating...\n");
            my_truncate(mip);
            freeOft->offset = 0;
            if (DEBUG) printf("Done truncating.\n");
            break;
        case 2:
            freeOft->offset = 0;
            break;
        case 3:
            freeOft->offset = mip->INODE.i_size;
            break;
        default:
            printf("Invalid mode.\n");
            return -1;
        ;
    }

    int freeFd = 0;
    while(running->fd[freeFd] != 0) {
        freeFd++;
    }

    running->fd[freeFd] = freeOft;

    if (mode == 0)
    {
        mip->INODE.i_atime = time(0L);
    }
    else 
    {
        mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
    }

    return freeFd;
}

int my_truncate(MINODE *mip)
{
    int lblock;
    int blk;
    while(blk = get_logical_block(mip, lblock++)) {
        bdalloc(dev, blk);
    }
    for (int i = 0; i < 14; i++) {
        mip->INODE.i_block[i] = 0;
    }
    mip->INODE.i_size = 0;
}

int my_close_file(int file) 
{
    if(file < 0 || file > 63 || running->fd[file] == 0) {
        printf("File descriptor not valid.\n");
        return 0;
    }

    OFT *oftp = running->fd[file];
    running->fd[file] = 0;
    oftp->refCount--;
    if(oftp->refCount == 0) return 0;

    iput(oftp->minodePtr);
    return 0;
}

int pfd() 
{
    printf(" fd     mode    offset      INODE  \n");
    printf("----    ----    ------    ---------\n");
    for(int i = 0; i < NFD; i++){
        if(running->fd[i] != 0) {
            OFT *oftp = running->fd[i];
            printf(" %2d       %d     %6d    [%d, %d]\n", i, oftp->mode, oftp->offset, dev, oftp->minodePtr->ino);
        }
    }
    printf("----------------------------------\n");
}