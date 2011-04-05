#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include "scull.h"

int create_dev(void)
{

        int result = -1; 
	dev_t dev;

        if(scull_major){
                dev = MKDEV(scull_major, scull_minor);
                result = register_chrdev_region(dev, scull_nr_devs, "scull");
        }else{
                result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
                scull_major = MAJOR(dev);
        }   

        if(result < 0){ 
                /* clean up */
                printk(KERN_WARNING "scull: Can't get major device number %d\n", scull_major);
        }   
	return result;

}

struct scull_qset *scull_follow(struct scull_dev *dev, int n)
{
	struct scull_qset *qs = dev->data;

	/* Allocate first qset explictly if needed */
	if(!qs){
		qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
		
		if(qs == NULL)
			return NULL; /* Don't care */
		
		memset(qs,0,sizeof(struct scull_qset));
	}

	/* Follow the list */
	while(n--){

		if(!qs->next){
			qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);

			if(qs->next == NULL)
				return NULL; /* Never mind */

			memset(qs->next,0,sizeof(struct scull_qset));
		}

		qs = qs->next;
		continue;
	}
	return qs;
}
