#include <linux/blkdev.h>

#include "mymodule_device_struct.h"

#ifndef H_MYBLOCKDRIVER_OPERATIONS
#define H_MYBLOCKDRIVER_OPERATIONS

extern struct block_device_operations mydevice_ops;
int setup_device(char* name, struct mydevice * dev, unsigned long nsect, int major_num);

#endif
