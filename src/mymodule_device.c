#include <linux/kernel.h> 		/* printk() */
#include <linux/errno.h>		/* error codes */
#include <linux/hdreg.h>		/* hd_geometry */
#include <linux/fs.h>			/* block_device, block_device_perations structs */
#include <linux/blkdev.h>		/* request_queue struct, blk_*() */
#include <linux/genhd.h>		/* gendisc */
#include <linux/vmalloc.h>		/* vmalloc(), vfree() */
#include <linux/kobject.h>

#include "mymodule_device_struct.h"
#include "mymodule_device.h"

#define KERNEL_SECTOR_SIZE 512
#define hardsect_size 512

/*
 * Device Operations
 */

static void mydevice_transfer(struct mydevice *dev, sector_t sector, unsigned long nsect, char *buffer, int write) {
	unsigned long offset = sector * hardsect_size;
	unsigned long nbytes = nsect * hardsect_size;

	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "mydevice: Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}
	if (write)
		memcpy(dev->data + offset, buffer, nbytes);
	else
		memcpy(buffer, dev->data + offset, nbytes);
}

static void mydevice_request(struct request_queue *q) {
	struct request *req;

	req = blk_fetch_request(q);
	while (req != NULL) {
		struct mydevice *dev = req->rq_disk->private_data;
		if ((req->cmd_type != REQ_TYPE_FS)) {
			printk (KERN_NOTICE "Skip non-FS request\n");
			__blk_end_request_all(req, -EIO);
			continue;
		}
		mydevice_transfer(dev, blk_rq_pos(req), blk_rq_cur_sectors(req), req->buffer, rq_data_dir(req));
		if ( ! __blk_end_request_cur(req, 0) ) {
			req = blk_fetch_request(q);
		}
	}
}

static int mydevice_getgeo(struct block_device * block_device, struct hd_geometry * geo) {
	struct mydevice* dev = block_device->bd_disk->private_data;
	/* Simulating some hd_geometry */
	geo->heads = 4;
	geo->sectors = 16;
	geo->cylinders = ( dev->size * hardsect_size / KERNEL_SECTOR_SIZE ) / geo->sectors / geo->heads ;
	geo->start = 0;
	return 0;
}

struct block_device_operations mydevice_ops = {
	.owner  = THIS_MODULE,
	.getgeo = mydevice_getgeo
};

/*
 * Constructor and destructor
 */

int setup_device(char* name, struct mydevice * dev, unsigned long nsect, int major_num){
	memset(dev, 0, sizeof(struct mydevice));
	dev->size = nsect*hardsect_size;
	dev->data = vmalloc(dev->size);
	if (dev->data == NULL){
		printk(KERN_ALERT "can't allocate memory for data\n");
		return -ENOMEM;
	}
	spin_lock_init(&dev->lock);
	dev->queue = blk_init_queue(mydevice_request, &dev->lock);
	if(dev->queue == NULL){
		vfree(dev->data);
		printk(KERN_ALERT "can't init queue\n");
		return 1; //TODO proper error
	}
	blk_queue_logical_block_size(dev->queue,hardsect_size);
	dev->queue->queuedata = dev;

	dev->gd = alloc_disk(16); //TODO proper minors
	if (dev->gd == NULL){
		blk_cleanup_queue(dev->queue);
		vfree(dev->data);
		printk(KERN_ALERT "can't allocate disk\n");
		return 1; // TODO proper error
	}
	dev->gd->major = major_num;
	dev->gd->first_minor = 0;
	dev->gd->fops = &mydevice_ops;
	dev->gd->queue=dev->queue;
	dev->gd->private_data = dev;
	strcpy(dev->gd->disk_name, name);
	set_capacity(dev->gd, nsect);
	add_disk(dev->gd);
	return 0;
}

void clenup_device( struct mydevice* dev){
	if ( dev->gd != NULL ) {
		del_gendisk(dev->gd);
		put_disk(dev->gd);
	}
	if ( dev->queue != NULL ) {
		blk_cleanup_queue(dev->queue);
	}
	if ( dev->data != NULL )
		vfree(dev->data);
}

