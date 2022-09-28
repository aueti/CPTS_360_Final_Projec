#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "util.h"
#include "alloc_dalloc.h"

int irmdir();
int rm_name(MINODE *mip, int ino, char *name);