#include <linux/init.h>			/* __init */
#include <linux/module.h>		/* module_*() */
#include <linux/slab.h>			/* kmalloc() */
#include <linux/kernel.h>		/* printk() */
#include <linux/fs.h>			/* regester_blkdev(), unregister_blkdev() */
#include <linux/errno.h>		/* error codes */

#include "mymodule_device.h"		/* setup_device(), cleanup_device() */
#include "mymodule_device_struct.h"	/* mydevice struct */

#define KERNEL_SECTOR_SIZE 512
#define SBULL_MINORS 16

static int major_num=0;
static int nsectors=204800;

static struct mydevice *Device = NULL;

static int __init myblockdevice_init(void){
	int res;
	major_num = register_blkdev(major_num, "myblockdevice");
	if (major_num <= 0) {
		printk(KERN_WARNING "myblockdevice: unable to get major number\n");
		return -EBUSY;
	}
	printk(KERN_INFO "myblockdevice registered\n");
	Device = kmalloc(sizeof (struct mydevice), GFP_KERNEL);
	if(Device == NULL){
		printk(KERN_ALERT "myblockdevice: kmalloc device failed \n");
		return -ENOMEM;
	}
	res = setup_device("mydevice",Device, nsectors, major_num);
	if( res!=0){
		kfree(Device);
		printk(KERN_ALERT "myblockdevice: setup_device failed \n");
		return res;
	}
	printk(KERN_INFO "disk setuped\n");
	return 0;
}

static void __exit myblockdevice_exit(void){
	if ( Device != NULL ){
		clenup_device(Device);
		printk(KERN_INFO "disk cleanuped\n");
		kfree(Device);
	}
	unregister_blkdev(major_num, "myblockdevice");
	printk(KERN_INFO "myblockdevice unregistered\n");
}

module_init(myblockdevice_init);
module_exit(myblockdevice_exit);
