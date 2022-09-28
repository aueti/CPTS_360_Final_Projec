#pragma once

#include "util.h"
#include "header.h"

int my_open_file(char *filePath, int mode);
int my_truncate(MINODE *mip);
int my_close_file(int file);
int pfd();