#include "cd_ls_pwd.h"

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

/************* cd_ls_pwd.c file **************/
int ls_file(DIR *dp, char *buffer)
{
  INODE *ip = &(iget(dev, dp->inode)->INODE);
  char tempBuffer[129];
  char nameBuffer[128];
  strncpy(nameBuffer, dp->name, dp->name_len);
  nameBuffer[dp->name_len] = 0;
  buffer[0] = '\0'; 
  char ftime[29];

  if ((ip->i_mode & 0xF000) == 0x8000) // if (S ISREG()) 
    sprintf(tempBuffer, "%c", '-'); 
  if ((ip->i_mode & 0xF000) == 0x4000) // if (S ISDIR()) 
    sprintf(tempBuffer, "%c",'d'); 
  if ((ip->i_mode & 0xF000) == 0xA000) // if (S ISLNK()) 
    sprintf(tempBuffer, "%c",'l'); 
  strcat(buffer, tempBuffer);
  for (int i=8; i >= 0; i-- )
  { 
    if (ip->i_mode & (1 << i)) // print r|w|x 
      sprintf(tempBuffer, "%c", t1[i]); 
    else 
      sprintf(tempBuffer, "%c", t2[i]); // or print 
    strcat(buffer, tempBuffer);
  } 
  sprintf(tempBuffer, "%4d ",ip->i_links_count); // link count 
  strcat(buffer, tempBuffer);

  sprintf(tempBuffer, "%4d ",ip->i_gid); // gid 
  strcat(buffer, tempBuffer);
  sprintf(tempBuffer, "%4d ",ip->i_uid); // uid 
  strcat(buffer, tempBuffer);
  
  // print time 
  time_t dateValue = ip->i_ctime;
  ctime_r(&dateValue, ftime);
  ftime[strlen(ftime) - 1] = 0; // kill \n at end 

  sprintf(tempBuffer, "%28s ",ftime); 
  strcat(buffer, tempBuffer);

  sprintf(tempBuffer, "%8d ",ip->i_size); // file size 
  strcat(buffer, tempBuffer);

  sprintf(tempBuffer, "%12s ", nameBuffer); // print file basename 
  strcat(buffer, tempBuffer);

  // print -> linkname if symbolic file 
  if ((ip->i_mode & 0xF000)== 0xA000)
  { 
    char linknameBuffer[60];
    ssize_t linkname = my_readlink(nameBuffer, linknameBuffer); // use my_readlink() to read linkname 
    linknameBuffer[linkname] = 0;
    if (DEBUG) printf("linkame = %ld\n", linkname);
    sprintf(tempBuffer, "-> %s", linknameBuffer); // print linked name 
    strcat(buffer, tempBuffer);
  } 

  //print inode numbers
  sprintf(tempBuffer, "[%d %2d]",dev, dp->inode); 
  strcat(buffer, tempBuffer);

  sprintf(tempBuffer, "\n");
  strcat(buffer, tempBuffer);
}

int ls_dir(MINODE *mip)
{
  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  while (cp < buf + BLKSIZE){
     char lsBuffer[256];
     ls_file(dp, lsBuffer);
     printf("%s", lsBuffer);

     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
}

int ls()
{
  if(strcmp(pathname, "") == 0){
    dev = running->cwd->dev;
    ls_dir(running->cwd);
  }
  else
  {
    int node = getino(pathname);
    if(node > 0)
    {
      if(DEBUG) printf("%d\n", node);
      MINODE *ip = iget(dev, node);
      if((ip->INODE.i_mode & 0xF000) == 0x4000)
      {
        ls_dir(ip);
      }
      else
      {
        printf("Cannot ls a file node.\n");
      }
    }
    else
    {
      printf("Directory does not exist.\n");
    }
  }
}

/************* cd_ls_pwd.c file **************/
int cd()
{
  if (!(my_access(pathname, 2)))
  { 
      printf("Incorrect permissions\n");
      return 0;
  }

  if(strcmp(pathname, "") == 0)
  {
    running->cwd = root;
    dev = root->dev;
    return 0;
  }

  // search for mip
  int ino = getino(pathname);
  MINODE* ip = iget(dev, ino);
  if(ino == 0)
  {
    printf("Could not locate directory.\n");
    return 0;
  }
  else
  {
    if((ip->INODE.i_mode & 0xF000) == 0x4000)
    {
      running->cwd = ip;
    }
    else
    {
      printf("Cannot cd to a file.\n");
    }
  }

  // set cwd
}

char *pwd(int start , MINODE* wd)
{
  if (wd == root){
    if(start)
    {
      printf("/\n");
    }
    else{
      printf("/");
    }
    return 0;
  }
  else {
    MINODE* parent = iget(dev, search(wd, ".."));

    if(parent == wd) {
      MOUNT *mountPoint = getMountFromDev(wd->dev);
      wd = mountPoint->mounted_inode;
      dev = wd->dev;
      ino = wd->ino;
      parent = iget(dev, search(wd, ".."));
    }

    pwd(0, parent);

    dev = wd->dev;
    int myino;
    char myname[64];
    findino(wd, &myino);
    findmyname(parent, myino, myname);
    if(start)
    {
      printf("%s\n", myname);
    }
    else{
      printf("%s/", myname);
    }
  }
}

