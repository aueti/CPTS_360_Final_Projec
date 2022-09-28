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
#include "util.h"

int ls_file(DIR *dp, char *buffer);
int ls_dir(MINODE *mip);
int ls();
int cd();
char *pwd(int start , MINODE* wd);
