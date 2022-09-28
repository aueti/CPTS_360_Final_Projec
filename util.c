/*********** util.c file ****************/
#include "util.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

int ownerP[3] = {0x0100, 0x0080, 0x0040};

int otherP[3] = {0x0010, 0x0008, 0x0004};

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  int i;
  char *s;
  if(DEBUG){printf("tokenize %s\n", pathname);}

  strcpy(gpath, pathname);   // tokens are in global gpath[ ]
  n = 0;

  s = strtok(gpath, "/");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }
  name[n] = 0;
  
  if(DEBUG)
  {
     for (i= 0; i<n; i++)
      printf("%s  ", name[i]);
     printf("\n");
  }
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
   initGlobalsForDev();
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, offset;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount && mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       if(DEBUG) printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk    = (ino-1)/8 + iblk;
       offset = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + offset;
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
  return 0;
}

void iput(MINODE *mip)
{
   initGlobalsForDev();
   int i, block, offset;
   char buf[BLKSIZE];
   INODE *ip;

   if (mip==0) 
      return;

   mip->refCount--;
 
   if (mip->refCount > 0) return;
   if (!mip->dirty)       return;
 
   /* write INODE back to disk */
   /**************** NOTE ******************************
      For mountroot, we never MODIFY any loaded INODE
                 so no need to write it back
   FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

   Write YOUR code here to write INODE back to disk
   *****************************************************/
   // get INODE of ino to buf    
   block = (mip->ino-1)/8 + iblk;
   offset = (mip->ino-1) % 8;

   get_block(dev, block, buf);
   ip = (INODE *)buf + offset;
   *ip = mip->INODE;
   put_block(dev, block, buf);
} 

int search(MINODE *mip, char *name)
{
   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   if(DEBUG) printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   if(DEBUG){ printf("  ino   rlen  nlen  name\n");}

   while (cp < sbuf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     if(DEBUG){
      if(DEBUG) printf("%4d  %4d  %4d    %s\n", 
            dp->inode, dp->rec_len, dp->name_len, dp->name);
     }
     if (strcmp(temp, name)==0){
        if(DEBUG){ printf("found %s : ino = %d\n", temp, dp->inode);}
        return dp->inode;
     }
     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   return 0;
}

int getino(char *pathname)
{
  int i, ino, blk, offset;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  if(DEBUG){printf("getino: pathname=%s\n", pathname);}
  if (strcmp(pathname, "/")==0)
      return 2;
  
  // starting mip = root OR CWD
  if (pathname[0]=='/') {
     mip = root;
     dev = root->dev;
     ino = 2;

  } else {
     mip = running->cwd;
     dev = mip->dev;
     ino = mip->ino;
     if (DEBUG) printf("Returning to cwd, dev is %d, and ino is %d\n", dev, ino);
  }

  mip->refCount++;         // because we iput(mip) later
  
  tokenize(pathname);

  for (i=0; i<n; i++){
     if(DEBUG){
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);
      }

      if(strcmp(name[i], "..") == 0 && search(mip, "..") == mip->ino && mip->dev != root->dev) {
         MOUNT *mountPoint = getMountFromDev(mip->dev);
         mip = mountPoint->mounted_inode;
         dev = mip->dev;
         ino = mip->ino;
      }

      ino = search(mip, name[i]);

      if (ino==0){
         iput(mip);
         if(DEBUG){
         printf("name %s does not exist\n", name[i]);}
         return 0;
      }
      iput(mip);
      mip = iget(dev, ino);

      if (DEBUG) printf("Inode %d has mounted value %d\n", mip->ino, mip->mounted);
      if(mip->mounted == 1) {
         dev = mip->mptr->dev;
         if (DEBUG) printf("Dev updated to %d\n", dev);
         mip = iget(dev, 2);
         ino = 2;
      }
   }

   iput(mip);
   return ino;
}

// These 2 functions are needed for pwd()
int findmyname(MINODE *parent, u32 myino, char myname[ ]) 
{
  // WRITE YOUR code here
  // search parent's data block for myino; SAME as search() but by myino
  // copy its name STRING to myname[ ]

   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   ip = &(parent->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;

   while (cp < sbuf + BLKSIZE){
     if (dp->inode == myino){
        strncpy(myname, dp->name, dp->name_len);
     }
     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   return 0;

}

int findino(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
{
  // mip points at a DIR minode
  // WRITE your code here: myino = ino of .  return ino of ..
  // all in i_block[0] of this DIR INODE.

   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;

   while (cp < sbuf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     if (strcmp(temp, ".")==0){
        *myino = dp->inode;
     }
     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   return 0;
}

int get_logical_block(MINODE *mip, int lblock)
{
   if(lblock < 12) {
      return mip->INODE.i_block[lblock];
   }
   else if(lblock < 12 + 256) {
      int buf[256];
      if(mip->INODE.i_block[12] == 0) return 0;
      get_block(dev, mip->INODE.i_block[12], (char *)buf);
      return  buf[lblock - 12];
   }
   else if(lblock < 12 + 256 + (256 * 256))
   {
      int buf[256];
      if(mip->INODE.i_block[13] == 0) return 0;
      get_block(dev, mip->INODE.i_block[13], (char *)buf);
      if(buf[(lblock - 12 - 256) / 256] == 0) return 0;
      int index = buf[(lblock - 12 - 256) / 256];
      get_block(dev, index, (char *)buf);
      return buf[(lblock - 12 - 256) % 256];
   }
}

int set_logical_block(MINODE *mip, int lblock, int value)
{
   if(lblock < 12) {
      mip->INODE.i_block[lblock] = value;
      if(DEBUG) printf("placed new block %d into direct block %d\n", value, lblock); 
   }
   else if(lblock < 12 + 256) {
      int buf[256];
      if(mip->INODE.i_block[12] == 0) mip->INODE.i_block[12] = balloc(dev);
      get_block(dev, mip->INODE.i_block[12], (char *)buf);
      buf[lblock - 12] = value;
      put_block(dev, mip->INODE.i_block[12], (char *)buf);
      if (DEBUG) printf("placed new indirect block\n"); 
   }
   else if(lblock < 12 + 256 + (256 * 256))
   {
      int buf[256];
      if(mip->INODE.i_block[13] == 0) mip->INODE.i_block[13] = balloc(dev);
      get_block(dev, mip->INODE.i_block[13], (char *)buf);
      if(buf[(lblock - 12 - 256) / 256] == 0) buf[(lblock - 12 - 256) / 256] = balloc(dev);
      put_block(dev, mip->INODE.i_block[13], (char *)buf);
      int index = buf[(lblock - 12 - 256) / 256];
      get_block(dev, index, (char *)buf);
      buf[(lblock - 12 - 256) % 256] = value;
      put_block(dev, index, (char *)buf);
      if(DEBUG) printf("placed new double indirect block\n"); 
   }
   mip->dirty = 1;
   iput(mip);
}

MOUNT *getMountFromDev(int dev){
   for(int i = 0; i < 8; i++) {
      if (mountTable[i].dev == dev){
         return &mountTable[i];
      }
   }
   return 0;
}

int initGlobalsForDev(){
      MOUNT *mount = getMountFromDev(dev);
      imap = mount->imap;
      bmap = mount->bmap;
      nblocks = mount->nblocks;
      ninodes = mount->ninodes;
      iblk = mount->iblk;
}

int my_access(char *filename, int mode)  // mode = r|w|x:
{
  int r;

  if (running->uid == 0)   // SUPERuser always OK
     return 1;

  // NOT SUPERuser: get file's INODE
  ino = getino(filename);
  mip = iget(dev, ino);

  if (mip->INODE.i_uid == running->uid)
      r = mip->INODE.i_mode & ownerP[mode]; //(check owner's rwx with mode);  // by tst_bit()
  else
      r = mip->INODE.i_mode & otherP[mode];//(check other's rwx with mode);  // by tst_bit()

  iput(mip);
  
  return r;
}

int my_mip_access(MINODE* mip, int mode)  // mode = r|w|x:
{
  int r;

  if (running->uid == 0)   // SUPERuser always OK
     return 1;

  // NOT SUPERuser: get file's INODE

  if (mip->INODE.i_uid == running->uid)
      r = mip->INODE.i_mode & ownerP[mode]; //(check owner's rwx with mode);  // by tst_bit()
  else
  {
      r = mip->INODE.i_mode & otherP[mode]; //(check other's rwx with mode);  // by tst_bit()
  }
  iput(mip);
  
  return r;
}

int isOwner(char *filename)
{
   int ino = getino(filename);
   MINODE *mip = iget(dev, ino);

   return mip->INODE.i_uid == running->uid;
} 