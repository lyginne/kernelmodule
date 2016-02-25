#include <linux/device.h>

#ifndef H_TEST_DRIVER
#define H_TEST_DRIVER

struct device_driver *get_mydriver(void);
void test_driver_unregister(void);
int test_driver_register(void);

#endif
