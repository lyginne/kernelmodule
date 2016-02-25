#include <linux/module.h>
#include <linux/device.h>

#include "test_driver.h"
#include "mymodule_main.h"
#include "mymodule_device.h"		/* setup_device(), cleanup_device() */
#include "mymodule_device_struct.h"	/* mydevice struct */
#include "test_bus.h"

#define KERNEL_SECTOR_SIZE 512
#define SBULL_MINORS 16

static int nsectors=204800;

static int user_creating=0;

static struct mydevice *device = NULL; //this should be a collection or kinda, but it's good with sufficient time


static struct device_driver this={
	.name = "test_driver",
	.owner = THIS_MODULE,
};

static void device_init(void){
	int res;
	device = kmalloc(sizeof (struct mydevice), GFP_KERNEL);
	if(device == NULL){
		printk(KERN_ALERT "myblockdevice: kmalloc device failed \n");
		return;
	}
	res = setup_device("mydevice",device, nsectors, major_num);
	if( res!=0){
		kfree(device);
		printk(KERN_ALERT "myblockdevice: setup_device failed \n");
		return;
	}
	printk(KERN_INFO "disk setuped\n");
}

static void device_exit(void){
	if ( device != NULL ){
		clenup_device(device);
		printk(KERN_INFO "disk cleanuped\n");
		kfree(device);
	}
}

static ssize_t driver_control_file_store(struct device_driver *driver, const char *buf,size_t count){
	// that function will parse teh command, but ain't nobydy got time for taht
	if(user_creating == 1 && device == NULL){
		device_init();
	}
	return count;
}

static DRIVER_ATTR(control_file, 0666, NULL, driver_control_file_store);


struct device_driver *get_mydriver(void){
	return &this;
}


int test_driver_register(void){
	int retval=0;
	retval=driver_register(&this);
	if(!retval){
		return retval;
	}
	retval=driver_create_file(&this,&driver_attr_control_file);
	if(!user_creating&&!retval){
		device_init();
	}
	return retval;
}
void test_driver_unregister(void){
	device_exit();
	driver_remove_file(&this,&driver_attr_control_file);
	driver_unregister(&this);
}

