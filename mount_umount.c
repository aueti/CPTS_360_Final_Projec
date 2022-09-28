#include "mount_umount.h"

MOUNT *getmptr(int dev)
{
    for(int i = 0; i < 8; i++) {
        if(mountTable[i].dev == dev) {
            return &(mountTable[0]);
        }
    }
    return 0;
}

int mount() {
    if (strcmp(pathname, "") == 0){
        printf("dev           name       mountname      \n");
        for(int i = 0; i < 8; i++) {
            if(mountTable[i].dev != 0) {
                printf("%3d%15s%16s\n", mountTable[i].dev, mountTable[i].name, mountTable[i].mount_name);
            }
        }
        return 0;
    }

    char buf[BLKSIZE];

    //Check if it is already mounted
    for(int i = 0; i < 8; i++) {
        if(mountTable[i].dev != 0 && strcmp(mountTable[i].name, pathname) == 0) {
            printf("File system already mounted.\n");
            return 0;
        }
    }

    int newFd;
    if((newFd = open(pathname, O_RDWR)) >= 0) {
        if (DEBUG) printf("Opened fd = %d\n", newFd);
        get_block(newFd, 1, buf);
        SUPER *sp = (SUPER *)buf;
        if(DEBUG) printf("Got super block, magic = %x\n", sp->s_magic);

        /* verify it's an ext2 file system ***********/
        if (sp->s_magic != 0xEF53){
            printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
            return 0;
        }     
        if(DEBUG) printf("EXT2 FS OK\n");
        
        //Get mount point
        int ino = getino(pathname2);
        MINODE *mip = iget(dev, ino);

        if ((mip->INODE.i_mode & 0xF000) != 0x4000) 
        {
            printf("%s is not a directory\n", pathname2);
            return 0;
        }

        int mountIndex;
        if((mountIndex = allocMount()) < 0) {
            printf("No free mount points.\n");
            return 0;
        }

        mountTable[mountIndex].mounted_inode = mip;
        strcpy(mountTable[mountIndex].name, pathname);
        strcpy(mountTable[mountIndex].mount_name, pathname2);
        mountTable[mountIndex].dev = newFd;
        mountTable[mountIndex].ninodes = sp->s_inodes_count;
        mountTable[mountIndex].nblocks = sp->s_blocks_count;

        get_block(newFd, 2, buf); 
        gp = (GD *)buf;

        mountTable[mountIndex].bmap = gp->bg_block_bitmap;
        mountTable[mountIndex].imap = gp->bg_inode_bitmap;
        mountTable[mountIndex].iblk = gp->bg_inode_table;

        mip->mounted = 1;
        mip->mptr = &(mountTable[mountIndex]);
    }

    return 0;
}

int allocMount() {
    for(int i = 0; i < 8; i++) {
        if(mountTable[i].dev == 0) {
            return i;
        }
    }
    return -1;
}

int umount() {
    for(int i = 0; i< 8; i++) {
        if(mountTable[i].dev != 0 && strcmp(mountTable[i].name, pathname) == 0) {
            for(int j = 0; j < NMINODE; j++) {
                if(minode[j].mounted == 1 && minode[j].mptr == &(mountTable[i])) {
                    minode[j].mounted = 0;
                    mountTable[i].dev = 0;
                    iput(&minode[j]);
                    return 0;
                }
            }
        }
    }
    printf("Umount failed.\n");
    return -1;
}