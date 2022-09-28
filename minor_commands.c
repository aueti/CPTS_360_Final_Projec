#include "minor_commands.h"

char *t3 = "xwrxwrxwr-------";
char *t4 = "----------------";

int my_stat()
{
    struct stat myst;
    int ino = getino(pathname);
    char ftime[29];

    if (ino == 0)
    {
        printf("Pathname is incorrect\n");
        return 0;
    }

    MINODE* mip = iget(dev, ino);
    printf("ino: %d mip->ino: %d\n", ino, mip->ino);

    printf("File: %s\n", pathname);
    printf("Size: %d \t Blocks: %d \n", mip->INODE.i_size, mip->INODE.i_blocks);
    printf("Device: %d  \t Inode: %d \t Links: %d\n", dev, ino, mip->INODE.i_links_count);

    printf("Access: (0644/");
    if ((mip->INODE.i_mode & 0xF000) == 0x8000) // if (S ISREG()) 
        printf("%c", '-'); 
    if ((mip->INODE.i_mode & 0xF000) == 0x4000) // if (S ISDIR()) 
        printf("%c",'d'); 
    if ((mip->INODE.i_mode & 0xF000) == 0xA000) // if (S ISLNK()) 
        printf("%c",'l'); 
    for (int i=8; i >= 0; i-- )
    { 
        if (mip->INODE.i_mode & (1 << i)) // print r|w|x 
            printf( "%c", t3[i]); 
        else 
            printf( "%c", t4[i]); // or print 
    } 

    // HOW MANY OF THESE TIMES DO I NEED?

    printf(") \t Uid: %d \t Gid: %d\n", mip->INODE.i_uid, mip->INODE.i_gid);

    time_t dateValueA = mip->INODE.i_atime;
    ctime_r(&dateValueA, ftime);
    ftime[strlen(ftime) - 1] = 0; // kill \n at end  

    printf("Access: %28s\n",ftime);

    time_t dateValueM = mip->INODE.i_mtime;
    ctime_r(&dateValueM, ftime);
    ftime[strlen(ftime) - 1] = 0; // kill \n at end  

    printf("Modify: %28s\n", ftime);
    
    time_t dateValueC = mip->INODE.i_ctime;
    ctime_r(&dateValueC, ftime);
    ftime[strlen(ftime) - 1] = 0; // kill \n at end  

    printf("Change: %28s\n", ftime);

    iput(mip);
}

int my_chmod(char* mode, char* pathname2)
{
    int index = 0;
    int ino = getino(pathname2);

    if (ino == 0)
    {
        printf("Pathname is incorrect\n");
        return 0;
    }

    MINODE* mip = iget(dev, ino);
    printf("ino: %d mip->ino: %d\n", ino, mip->ino);
    
    if (mode[0] == '+')
    {
        printf("Adding Permissions ...\n");

        while(mode[index])
        {
            if (mode[index] == 'x')
            {
                printf("Adding Executable Permissions ...\n");
                mip->INODE.i_mode |= S_IEXEC;
            }
            else if(mode[index] == 'r')
            {
                printf("Adding Reading Permissions ...\n");
                mip->INODE.i_mode |= S_IREAD;
            }
            else if(mode[index] == 'w')
            {
                printf("Adding Writing Permissions ...\n");
                mip->INODE.i_mode |= S_IWRITE;
            }

            index++;
        }
    }
    else if (mode[0] == '-')
    {
        printf("Removing Permissions ...\n");

        while(mode[index])
        {
            if (mode[index] == 'x')
            {
                printf("Removing Executable Permissions ...\n");
                mip->INODE.i_mode &= ~(S_IEXEC);
            }
            else if(mode[index] == 'r')
            {
                printf("Removing Reading Permissions ...\n");
                mip->INODE.i_mode &= ~(S_IREAD);
            }
            else if(mode[index] == 'w')
            {
                printf("Removing Writing Permissions ...\n");
                mip->INODE.i_mode &= ~(S_IWRITE);
            }

            index++;
        }
    }
    else
    {
        printf("Invalid mode\n");
        return 0;
    }

    mip->dirty = 1;

    iput(mip);
}

int my_utime()
{
    int ino = getino(pathname);

    if (ino == 0)
    {
        printf("Pathname is incorrect\n");
        return 0;
    }

    MINODE* mip = iget(dev, ino);
    printf("ino: %d mip->ino: %d\n", ino, mip->ino);

    mip->INODE.i_atime = time(0L);

    mip->dirty = 1;

    iput(mip);
}