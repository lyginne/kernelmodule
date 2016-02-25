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

#define KERNEL_SECTOR_SIZE 512
#define SBULL_MINORS 16

static int major_num=0;
static int nsectors=204800;
static struct kobject* kobj;

static int userCreating=0;
module_param(userCreating, int, 0);

static struct mydevice *Device = NULL;

static void device_init(void){
	int res;
	Device = kmalloc(sizeof (struct mydevice), GFP_KERNEL);
	if(Device == NULL){
		printk(KERN_ALERT "myblockdevice: kmalloc device failed \n");
		return;
	}
	res = setup_device("mydevice",Device, nsectors, major_num);
	if( res!=0){
		kfree(Device);
		printk(KERN_ALERT "myblockdevice: setup_device failed \n");
		return;
	}
	printk(KERN_INFO "disk setuped\n");
}

static void device_exit(void){
	if ( Device != NULL ){
		clenup_device(Device);
		printk(KERN_INFO "disk cleanuped\n");
		kfree(Device);
	}
}

static void driver_kobject_init(const char* name){
	kobj=kobject_create_and_add("mydriver", kernel_kobj); //parent is not proper, change it when figure out the proper one
	if(kobj){
		printk(KERN_WARNING "myblockdevice: kobj is null\n");
	}
	else{
		printk(KERN_WARNING "myblockdevice: kobj is not null\n");
	}
}

static void driver_kobject_exit(void){
	kobject_put(kobj);
}

static int __init myblockdevice_init(void){
	major_num = register_blkdev(major_num, "myblockdevice");
	if (major_num <= 0) {
		printk(KERN_WARNING "myblockdevice: unable to get major number\n");
		return -EBUSY;
	}
	driver_kobject_init("mydriver");
	if(!userCreating){
		device_init();
	}
	printk(KERN_INFO "myblockdevice registered\n");
	return 0;
}

static void __exit myblockdevice_exit(void){
	device_exit();
	driver_kobject_exit();
	unregister_blkdev(major_num, "myblockdevice");
	printk(KERN_INFO "myblockdevice unregistered\n");
}

MODULE_LICENSE("GPL v2");
module_init(myblockdevice_init);
module_exit(myblockdevice_exit);
