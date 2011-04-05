#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h> /* Parameters definition */
#include <linux/kdev_t.h>
#include "scull.h"

unsigned int scull_major = SCULL_MAJOR;
unsigned int scull_minor = SCULL_MINOR;
unsigned int scull_nr_devs = 4; /* number of devices */
unsigned int scull_quantum = SCULL_QUANTUM;
unsigned int scull_qset = SCULL_QSET;

/* Create devices */
static void scull_setup_cdev(struct scull_dev *dev, int index)  /*dev struct not yet initialized into the code. FIXME*/ 
{
	int err, devno = MKDEV(scull_major, scull_minor + index);

	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	err = cdev_add(&dev->cdev, devno, 1);
		 /*FIXME: should fail gracefully*/
	if(err)
		printk(KERN_NOTICE "Error %d adding scull%d",err, index);
}
	


static int scull_init(void)
{
	
	struct scull_dev *dev;
	printk("Starting scull module...\n");
	create_dev();

	dev = kmalloc(sizeof(struct scull_dev*),GFP_KERNEL);
	scull_setup_cdev(dev,5);

	
	return 0;
}

static void scull_exit(void)
{
	printk("Unloading scull module...\n");
}

module_init(scull_init);
module_exit(scull_exit);




/* Module information */

MODULE_LICENSE("GPL");
