#include "mkdir_creat.h"

int imkdir()
{
    strcpy(child, basename(pathname));
    strcpy(parent, dirname(pathname));

    if(DEBUG) printf("Dirname: %s Basename: %s\n", parent, child);

    pino = getino(parent);
    pmip= iget(dev, pino);

    if(DEBUG) printf("pino: %d pmip->pino: %d\n", pino, pmip->ino);

    if(DEBUG) printf("Checking if %s is a dir...\n", parent);
    if ((pmip->INODE.i_mode & 0xF000) != 0x4000) 
    {
        printf("%s is not a directory\n", parent);
        return 0;
    }

    if(DEBUG) printf("Checking if %s already exists in %s\n", child, parent);
    if(search(pmip, child) != 0)
    {
        printf("Dir %s already exists in %s\n", child, parent);
        return 0;
    }

    ikmkdir(pmip, child);
}

int ikmkdir(MINODE* pmip, char* basename)
{
    int ino = ialloc(dev);
    int blk = balloc(dev);

    MINODE* mip = iget(dev, ino);

    INODE *ip = &mip->INODE;
    ip->i_mode = 0x41ED; // 040755: DIR type and permissions
    ip->i_uid = running->uid; // owner uid
    ip->i_gid = running->gid; // group Id
    ip->i_size = BLKSIZE; // size in bytes
    ip->i_links_count = 2; // links count=2 because of . and ..
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2; // LINUX: Blocks count in 512 byte chunks
    ip->i_block[0] = blk; // new DIR has one data block

    for(int i = 1; i < 14; i++ )
    {
        ip->i_block[i] = 0;
    }

    mip->dirty = 1; // mark minode dirty

    iput(mip);

    pmip->INODE.i_links_count++;
    pmip->dirty = 1;

    iput(mip);

    enter_name(mip, ino, ".", 1);
    enter_name(mip, pmip->ino, "..", 1);
    enter_name(pmip, ino, child, 1);
}

int enter_name(MINODE *mip, int ino, char *name, int isDir)
{
   get_block(dev, mip->INODE.i_block[0], buf);
   DIR *dp = (DIR *)buf;
   char *cp = buf;

   if(DEBUG) printf("Fetched dir block...\n");

   if(*cp == 0)
   {
      if(DEBUG) printf("In new block\n");
      dp->inode = ino;
      strcpy(dp->name, name);
      dp->name[strlen(name)] = 0;
      dp->name_len = strlen(name);
      dp->file_type = isDir? 'd' : 'r';
      dp->rec_len = BLKSIZE;

      put_block(dev, mip->INODE.i_block[0], buf);

      return 1;
   }

   while (cp + dp->rec_len < buf + BLKSIZE){
      if(DEBUG) printf("scanning ino %d, reclen = %d\n", dp->inode, dp->rec_len);
      cp += dp->rec_len;
      dp = (DIR *)cp;
   }

   if(DEBUG) printf("Found empty entry\n");

   int oldRecLength = dp->rec_len;
   dp->rec_len = sizeof(DIR) + dp->name_len - 1;
   dp->rec_len += 4 - (dp->rec_len % 4);
   int newRecLength = dp->rec_len;
   cp += dp->rec_len;
   dp = (DIR *)cp;

   dp->inode = ino;
   strcpy(dp->name, name);
   dp->name[strlen(name)] = 0;
   dp->name_len = strlen(name);
   dp->file_type = isDir? 'd' : 'r';
   dp->rec_len = oldRecLength - newRecLength;

   put_block(dev, mip->INODE.i_block[0], buf);

   return 1;
}

int my_creat(char *pathname)
{
    strcpy(child, basename(pathname));
    strcpy(parent, dirname(pathname));

    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);

    if((pmip->INODE.i_mode & 0xF000) != 0x4000)
    {
        printf("Pathname does not lead to a directory.\n");
        return 0;
    }

    if(search(pmip, child) != 0)
    {
        printf("File already exists.\n");
        return 0;
    }

    int ino = ialloc(dev);
    MINODE *mip = iget(dev, ino);
    mip->INODE.i_mode = 0x0000;
    mip->INODE.i_mode |= 0x8000;
    mip->INODE.i_mode |= 0644;
    mip->INODE.i_links_count = 1;
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->INODE.i_ctime = mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
    mip->INODE.i_blocks = 2;
    for(int i = 0; i < 15; i++) mip->INODE.i_block[i] == 0;


    mip->dirty = 1;
    iput(mip);

    //Add new inode to parent
    enter_name(pmip, ino, child, 0);
    pmip->dirty = 1;
    iput(pmip);
}