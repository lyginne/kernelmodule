#include <linux/module.h>
#include <linux/device.h>
#include <linux/types.h>

#include "test_driver.h"
#include "mymodule_main.h"
#include "mymodule_device.h"		/* setup_device(), cleanup_device() */
#include "mymodule_device_struct.h"	/* mydevice struct */
#include "test_bus.h"

#define KERNEL_SECTOR_SIZE 512
#define SBULL_MINORS 16

static int default_device_size_in_kbytes = 102400; //100 Mb - default size

int user_creating=0;
module_param(user_creating, int, 0);
MODULE_PARM_DESC(user_creating, "flag that allows user to create devices via sysfs");

//Это слишком тупой путь. Надо бы сделать коллекцию и все хранить в ней, а по=хорошему воспользоваться встроенной
static struct mydevice *device = NULL;

static struct device_driver this={
	.name = "test_driver",
	.owner = THIS_MODULE,
};

static int device_init(char* name, size_t device_size_in_kbytes){
	unsigned long nsectors;
	int res;
	printk( KERN_INFO MODULE_PREFIX "intent to make new device: %s %zu\n", name, device_size_in_kbytes);
	nsectors = device_size_in_kbytes<<1;
	device = kmalloc(sizeof (struct mydevice), GFP_KERNEL);
	if(device == NULL){
		printk(KERN_ALERT MODULE_PREFIX "kmalloc device failed \n");
		return -ENOMEM;
	}
	res = setup_device(name,device, nsectors, major_num);
	if(res){
		kfree(device);
		device=NULL;
		printk(KERN_ALERT MODULE_PREFIX "setup_device failed \n");
		return res;
	}
	printk(KERN_INFO "disk setuped\n");
	return 0;
}

static void device_exit(void){
	if ( device != NULL ){
		clenup_device(device);
		printk(KERN_INFO MODULE_PREFIX "disk cleanuped\n");
		kfree(device);
	}
}

static ssize_t driver_control_file_store(struct device_driver *driver, const char *buf,size_t count){
	//we should catch overflow somewhere
	char* command;
	char* name;
	size_t device_size_in_kbytes;
	int retval;
	
	command = kmalloc(count, GFP_KERNEL);
	if(!command){
		printk(KERN_ALERT MODULE_PREFIX "can't alloc command\n");
		return count;
	}
	name = kmalloc(count, GFP_KERNEL);//TODO check name for overflow
	if(!name){
		printk(KERN_ALERT MODULE_PREFIX "can't alloc name\n");
		kfree(command);
		return count;
	}
	retval = sscanf(buf, "%s", command);
	if(retval!=1){
		printk(KERN_WARNING MODULE_PREFIX "can't parse command\n");
		kfree(command);
		kfree(name);
		return count;
	}
	if(!strcmp(command, "create")){
		retval = sscanf(buf, "%*s %s %zu", name, &device_size_in_kbytes);
		if(retval!=2){
			printk(KERN_WARNING MODULE_PREFIX "can't parse name or size\n");
			kfree(command);
			kfree(name);
			return count;
		}
		if(!device){
			retval = device_init(name, device_size_in_kbytes);
			kfree(command);
			kfree(name);
			return count;
		}
		// we actualy should make some sort of collection, but not now
		printk(KERN_WARNING MODULE_PREFIX "device exist, skiping\n");
	}
	else{
		printk(KERN_WARNING MODULE_PREFIX "unknown command\n");
	}
	kfree(command);
	kfree(name);
	return count;	
}

static DRIVER_ATTR(control_file, 0666, NULL, driver_control_file_store);


struct device_driver *get_mydriver(void){
	return &this;
}


int test_driver_register(void){
	int retval=0;
	retval=driver_register(&this);
	if(retval){
		printk( KERN_ALERT MODULE_PREFIX "driver_register error\n");
		return retval;
	}
	// if user_creating disallowed, then we auto-create device and do not create driver_attr_contol_file
	if(!user_creating){
		device_init("test_device0", default_device_size_in_kbytes);
	}
	else{
		retval=driver_create_file(&this,&driver_attr_control_file);
		if(retval){
			printk( KERN_ALERT MODULE_PREFIX "driver_attr_control_file won't create\n");
			return retval;
		}
		printk( KERN_ALERT MODULE_PREFIX "driver_attr_control_file created\n");
	}
	return retval;
}
void test_driver_unregister(void){
	if(device)
		device_exit();
	if(user_creating)
		driver_remove_file(&this,&driver_attr_control_file);
	driver_unregister(&this);
}
