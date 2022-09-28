#pragma once

#include "type.h"
#include "header.h"

MOUNT *getmptr(int dev);
int mount();
int allocMount();
int umount();