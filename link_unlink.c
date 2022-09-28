#include "link_unlink.h"

int my_link()
{
    int oino = getino(pathname);
    if (DEBUG) printf("INO = %d\n", oino);
    MINODE *omip = iget(dev, oino);

    if((omip->INODE.i_mode & 0xF000) == 0x4000)
    {
        printf("Cannot link from a DIR\n");
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
    if(DEBUG) printf("Entering name\n");

    enter_name(pmip, oino, child, 0);
    if(DEBUG) printf("Entered new DIR\n");

    omip->INODE.i_links_count++;
    omip->dirty = 1;
    iput(omip);
    iput(pmip);

    return 1;
}

int my_unlink()
{
    if (!(isOwner(pathname)))
    { 
        printf("Incorrect permissions - not file owner\n");
        return 0;
    }

    int ino = getino(pathname);
    MINODE *mip = iget(dev, ino);

    if((mip->INODE.i_mode & 0xF000) == 0x4000)
    {
        printf("Cannot unlink a directory\n");
        return 0;
    }

    strcpy(child, basename(pathname));
    strcpy(parent, dirname(pathname));
    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);
    if(DEBUG) printf("Removing name\n");

    rm_name(pmip, ino, child);
    if(DEBUG) printf("Removed DIR\n");

    pmip->dirty = 1;
    iput(pmip);

    mip->INODE.i_links_count--;
    if(mip->INODE.i_links_count > 0)
    {
        mip->dirty = 1;
    }
    else
    {
        // Free everything
    }
    iput(mip);
}