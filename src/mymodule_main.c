#include <linux/init.h>			/* __init */
#include <linux/module.h>		/* module_*() */
#include <linux/slab.h>			/* kmalloc() */
#include <linux/kernel.h>		/* printk() */
#include <linux/fs.h>			/* regester_blkdev(), unregister_blkdev() */
#include <linux/errno.h>		/* error codes */
#include <linux/kobject.h>		/* all kobject stuff */
#include <linux/device.h>		/* sysfs_dev_block_kobj */
#include <linux/sysfs.h>
#include <linux/string.h>

#include "mymodule_device.h"		/* setup_device(), cleanup_device() */
#include "mymodule_device_struct.h"	/* mydevice struct */
#include "mymodule_main.h"
#include "test_bus.h"


int major_num=0;

static int __init myblockdevice_init(void){
	major_num = register_blkdev(major_num, "myblockdevice");
	if (major_num <= 0) {
		printk(KERN_WARNING "myblockdevice: unable to get major number\n");
		return -EBUSY;
	}
	//test_bus_register();
	printk(KERN_INFO "myblockdevice registered\n");
	return 0;
}

static void __exit myblockdevice_exit(void){
	//test_bus_unregister();
	unregister_blkdev(major_num, "myblockdevice");
	printk(KERN_INFO "myblockdevice unregistered\n");
}

MODULE_LICENSE("GPL v2");
module_init(myblockdevice_init);
module_exit(myblockdevice_exit);
