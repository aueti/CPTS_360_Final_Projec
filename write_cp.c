#include "write_cp.h"

int mywrite(int fd, char buf[], int nbytes)
{
    OFT *oftp = running->fd[fd];
    int lblk = oftp->offset / BLKSIZE;
    int startByte = oftp->offset % BLKSIZE;
    int remaining = nbytes;

    char blockBuf[BLKSIZE];
    int blk = get_logical_block(oftp->minodePtr, lblk);

    if(blk == 0) {
        blk = balloc(dev);
        set_logical_block(oftp->minodePtr, lblk, blk);
    }

    get_block(dev, blk, blockBuf);

    if(BLKSIZE - startByte >= remaining) {
        if (DEBUG) printf("Copied a partial block: %d bytes.\n", remaining);
        memcpy(blockBuf + startByte, buf, remaining);
        remaining = 0;
    } else {
        if (DEBUG) printf("Finished a partial block: %d bytes.\n", (BLKSIZE-startByte));
        memcpy(blockBuf + startByte, buf, BLKSIZE - startByte);
        remaining -= (BLKSIZE - startByte);
        buf += (BLKSIZE - startByte);
    }

    put_block(dev, blk, blockBuf);

    while (remaining > 0) {
        lblk++;
        blk = get_logical_block(oftp->minodePtr, lblk);
        if(blk == 0) {
            blk = balloc(dev);
            set_logical_block(oftp->minodePtr, lblk, blk);
        }

        get_block(dev, blk, blockBuf);

        if (remaining < BLKSIZE) {
            if (DEBUG) printf("Copied a partial block: %d bytes.\n", remaining);
            memcpy(blockBuf, buf, remaining);
            remaining = 0;
        } else {
            if (DEBUG) printf("Read full block.\n");
            memcpy(blockBuf, buf, BLKSIZE);
            remaining -= BLKSIZE;
            buf += BLKSIZE;
        }
    }

    //Extend the i_size if relevant to do so
    oftp->minodePtr->INODE.i_size = oftp->minodePtr->INODE.i_size >= oftp->offset + nbytes 
        ? oftp->minodePtr->INODE.i_size 
        : oftp->offset + nbytes;
    oftp->offset += nbytes;
    oftp->minodePtr->dirty = 1;
    iput(oftp->minodePtr);
    return nbytes;
}

int my_cp()
{
    if(getino(pathname) == 0) {
        printf("Source file does not exist.\n");
        return 0;
    }

    if(getino(pathname2) == 0) {
        my_creat(pathname2);
    }

    if (DEBUG) printf("Opening files.\n");
    int sourceFile = my_open_file(pathname, 0);
    int destFile = my_open_file(pathname2, 1);

    if(sourceFile == -1 || destFile == -1) {
        printf("Could not open target files.\n");
        return 0;
    }
    
    if (DEBUG) printf("Ready to read.\n");
    int n;
    char buf[BLKSIZE];
    while(n = myread(sourceFile, buf, BLKSIZE)  ){
        mywrite(destFile, buf, n);
    }

    my_close_file(sourceFile);
    my_close_file(destFile);

    return 1;
}