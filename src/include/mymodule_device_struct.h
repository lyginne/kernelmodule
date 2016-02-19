#include <linux/spinlock_types.h>

#ifndef H_MYBLOCKDRIVER_DEVICE
#define H_MYBLOCKDRIVER_DEVICE

struct mydevice {
	int hardware_sectsize;
        int size;                       /* Device size in sectors */
        u8 *data;                       /* The data array */
        spinlock_t lock;                /* For mutual exclusion */
        struct request_queue *queue;    /* The device request queue */
        struct gendisk *gd;             /* The gendisk structure */
};

#endif
