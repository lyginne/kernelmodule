#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>	/* u8 type */
#include <linux/slab.h>		/*kmalloc()*/
#include <linux/genhd.h>
#include <linux/blkdev.h>

#include "mymodule_device.h"
#include "mymodule_device_struct.h"

#define KERNEL_SECTOR_SIZE 512
#define SBULL_MINORS 16

static int major_num=0;
static int nsectors=204800;
static int hardsect_size = 512;

static struct mydevice *Device = NULL;

static int __init myblockdevice_init(void){
	major_num = register_blkdev(major_num, "myblockdevice");
	if (major_num <= 0) {
		printk(KERN_WARNING "myblockdevice: unable to get major number\n");
		return -EBUSY;
	}
	Device = kmalloc(sizeof (struct mydevice), GFP_KERNEL);
	if(Device == NULL){
		printk(KERN_ALERT "myblockdevice: kmalloc device failed \n");
		return -ENOMEM;
	}
	if(setup_device("mydevice",Device, nsectors, major_num)){
		kfree(Device);
		printk(KERN_ALERT "myblockdevice: setup_device failed \n");
		return 1; //change to number
	}
	printk(KERN_ALERT "myblockdevice: init succed\n");
	return 0;
}

static void __exit myblockdevice_exit(void){
	if (Device!=NULL){
		if (Device->gd) {
			del_gendisk(Device->gd);
			put_disk(Device->gd);
		}
		if (Device->queue) {
			blk_cleanup_queue(Device->queue);
		}
		if (Device->data)
			vfree(Device->data);
	}
	unregister_blkdev(major_num, "myblockdevice");
	kfree(Device);
	printk(KERN_ALERT "myblockdevice: exit succed\n");
}

module_init(myblockdevice_init);
module_exit(myblockdevice_exit);
