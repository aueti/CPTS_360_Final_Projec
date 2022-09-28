/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
#include "header.h"

extern MINODE *iget();

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    p->cwd = iget(dev, 2);
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  if(DEBUG) printf("mount_root()\n");

  mountTable[0].dev = dev;
  mountTable[0].ninodes = ninodes;
  mountTable[0].nblocks = nblocks;
  mountTable[0].imap = imap;
  mountTable[0].bmap = bmap;
  mountTable[0].iblk = iblk;

  strcpy(mountTable[0].name, "diskimage");
  root = iget(dev, 2);
}

char *disk = "diskimage";
int main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];

  printf("checking EXT2 FS ....");
  if ((fd = open(argv[1], O_RDWR)) < 0){
    printf("open %s failed\n", argv[1]);
    exit(1);
  }

  dev = fd;    // global dev same as this fd   

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

  mount_root();
  init(); 
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  proc[1].uid = 1;

  // WRTIE code here to create P1 as a USER process
  
  while(1){
    pathname[0] = 0;
    pathname2[0] = 0;
    printf("input command : ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;

    sscanf(line, "%s %s %s", cmd, pathname, pathname2);
    if(DEBUG) printf("cmd=%s pathname=%s pathname2=%s\n", cmd, pathname, pathname2);
  
    if (strcmp(cmd, "ls")==0)
       ls();
    else if (strcmp(cmd, "cd")==0)
       cd();
    else if (strcmp(cmd, "pwd")==0)
       pwd(1, running->cwd);
    else if (strcmp(cmd, "link") == 0)
      my_link();
    else if (strcmp(cmd, "quit")==0)
       quit();
    else if(strcmp(cmd, "unlink") == 0)
      my_unlink();
    else if(strcmp(cmd, "cp") == 0)
      my_cp();
    else if(strcmp(cmd, "creat") == 0)
      my_creat(pathname);
    else if (strcmp(cmd, "symlink") == 0)
      my_symlink();
    else if (strcmp(cmd, "mkdir") == 0)
      imkdir();
    else if (strcmp(cmd, "rmdir") == 0)
      irmdir();
    else if (strcmp(cmd, "stat") == 0)
      my_stat();
    else if (strcmp(cmd, "chmod") == 0)
      my_chmod(pathname, pathname2);
    else if(strcmp(cmd, "utime") == 0)
      my_utime();
    else if(strcmp(cmd, "cat") == 0)
      mycat(pathname);
    else if(strcmp(cmd, "pfd") == 0)
      pfd();
    else if(strcmp(cmd, "mount") == 0)
      mount();
    else if(strcmp(cmd, "umount") == 0)
      umount();
    else if(strcmp(cmd, "su") == 0)
      running = (running == &proc[0]) ? &proc[1] : &proc[0];
  }
}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}
