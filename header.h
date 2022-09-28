#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "link_unlink.h"
#include "type.h"
#include "cd_ls_pwd.h"
#include "minor_commands.h"
#include "mkdir_creat.h"
#include "rmdir.h"
#include "symlink.h"
#include "open_close.h"
#include "write_cp.h"
#include "mount_umount.h"

#define DEBUG 0

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[64];  // assume at most 64 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[128], pathname2[128];

int init();
int mount_root();
int quit();

