#include "rmdir.h"

int irmdir()
{
   if (!(isOwner(pathname)))
   { 
      printf("Incorrect permissions - not directory owner\n");
      return 0;
   }

   ino = getino(pathname);
   strcpy(child, basename(pathname));
   strcpy(parent, dirname(pathname));

    mip= iget(dev, ino);

    if(DEBUG) printf("ino: %d mip->ino: %d\n", ino, mip->ino);

    if(DEBUG) printf("Checking if %s is a dir...\n", child);
    if ((mip->INODE.i_mode & 0xF000) != 0x4000) 
    {
        printf("%s is not a directory\n", child);
        return 0;
    }

    if (mip->INODE.i_links_count > 2)
    {
        printf("Can not remove a directory that isn't empty\n");
        return 0;
    }

    if ((strcmp(child, ".") == 0) || (strcmp(child, "..") == 0))
    {
        printf("Can not remove %s directory\n", child);
        return 0;
    }

    pino = getino(parent);
    pmip = iget(mip->dev, pino);
    if(DEBUG) printf("ino: %d mip->ino: %d\n", pino, pmip->ino);

    findmyname(pmip, ino, nodeName);
    if(DEBUG) printf("name: %s\n", nodeName);

    rm_name(pmip, ino, nodeName);

    pmip->INODE.i_links_count--;
    pmip->dirty = 1;

    iput(mip);

    //bdalloc(mip->dev, mip->INODE.i_block[0]);
    idalloc(mip->dev, mip->ino); //FREE DATA
    iput(mip);
}

int rm_name(MINODE *mip, int ino, char *name)
{
   get_block(dev, mip->INODE.i_block[0], buf);
   DIR *dp = (DIR *)buf;
   char *cp = buf;
   DIR *lastdp;
   int deletedLength;

   char newBuf[BLKSIZE];
   char *newcp = newBuf;

   if(DEBUG) printf("Fetched dir block...\n");

   int foundTarget = 0;
   if (DEBUG) printf("Looking for inode = %d name = %s\n", ino, name);
   while (cp + dp->rec_len < buf + BLKSIZE){
      if(DEBUG) printf("scanning ino %d, reclen = %d\n", dp->inode, dp->rec_len);

      if(dp->inode == ino && strncmp(dp->name, name, dp->name_len) == 0){
         deletedLength = dp->rec_len;
         if (DEBUG) printf("Deleted %d bytes partway.\n", deletedLength);
      }
      else
      {
         lastdp = (DIR*)newcp;
         memcpy(newcp, cp, dp->rec_len);
         newcp += dp->rec_len;
      }

      cp += dp->rec_len;
      dp = (DIR *)cp;
   }

   if(strncmp(dp->name, name, dp->name_len) == 0)
   {
      lastdp->rec_len += dp->rec_len+1;
      if (DEBUG) printf("Deleted node at end\n");
   }
   else
   {
      memcpy(newcp, cp, dp->rec_len);
      ((DIR*)newcp)->rec_len += deletedLength;
   }

   put_block(dev, mip->INODE.i_block[0], newBuf);
}