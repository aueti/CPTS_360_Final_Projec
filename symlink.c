#include "symlink.h"

int my_readlink(char *file, char buffer[])
{
    int ino = getino(file);
    MINODE* mip = iget(dev, ino);

    if((mip->INODE.i_mode & 0xF000) != 0xA000)
    {
        printf("Target is not a link.\n");
        return 0;
    }

    memcpy(buffer, mip->INODE.i_block, mip->INODE.i_size);
    return mip->INODE.i_size;
}

int my_symlink()
{
    int oino = getino(pathname);
    if (DEBUG) printf("INO = %d\n", oino);
    MINODE *omip = iget(dev, oino);

    if(getino(pathname) == 0)
    {
        printf("Target file does not exist.\n");
        return 0;
    }

    if(getino(pathname2) != 0)
    {
        printf("File '%s' already exists.\n", pathname2);
        return 0;
    }

    strcpy(child, basename(pathname2));
    strcpy(parent, dirname(pathname2));
    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);

    int ino = ialloc(dev);
    MINODE *mip = iget(dev, ino);
    mip->INODE.i_mode = 0x0000;
    mip->INODE.i_mode |= 0xA000;
    mip->INODE.i_mode |= 0644;
    mip->INODE.i_links_count = 1;
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->INODE.i_ctime = mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
    mip->INODE.i_blocks = 2;
    strcpy(child2, basename(pathname));
    memcpy((char *)(mip->INODE.i_block), child2, strlen(child2));
    mip->INODE.i_size = strlen(child2);  

    mip->dirty = 1;
    iput(mip);

    //Add new inode to parent
    enter_name(pmip, ino, child, 0);
    pmip->dirty = 1;
    iput(pmip);
    return 0;
}