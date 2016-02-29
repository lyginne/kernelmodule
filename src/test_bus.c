#include <linux/device.h>
#include <linux/module.h>

#include "mymodule_main.h"
#include "test_bus.h"
#include "test_driver.h"

static struct bus_type test_bus = {
	.name = "test_bus",
};

int test_bus_register(void){
	int retval=0;
	retval = bus_register(&test_bus);
	if (retval < 0) {
		printk(KERN_WARNING "sysfs: error register bus: %d\n", retval);
		return retval;
     	}
	get_mydriver()->bus=&test_bus;
	retval = test_driver_register();
	if(retval){
		printk(KERN_WARNING "sysfs: driver register error");
		return retval;
	}
	printk(KERN_WARNING "sysfs: bus register succed");
	
	return 0;
}

void test_bus_unregister(void){
	test_driver_unregister();
	bus_unregister(&test_bus);
}

