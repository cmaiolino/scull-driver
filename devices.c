#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
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

int scull_read_procmem(char *buf, char **start, off_t offset,
		int count, int *eof, void *data)
{
	int i, j, len = 0;
	int limit = count - 80; /* Don't print more than this */
	
	len += sprintf(buf,"\nlimit value: %d\n\n",limit);
	for(i = 0; i < scull_nr_devs && len <= limit; i++){
		struct scull_dev *d = &scull_devices[i];
		struct scull_qset *qs = d->data;
		
		if(down_interruptible(&d->sem))
			return -ERESTARTSYS;

		len += sprintf(buf+len,"\nDevice: %i: qset %i, q %i, sz %li\n",
			i, d->qset, d->quantum, d->size);
		
		for(;qs && len <= limit; qs = qs->next){ /* Scan the list */
			len += sprintf(buf+len, " item at %p, qset at %p\n",
				qs, qs->data);
	
			if(qs->data && !qs->next){ /* dump only the last item */
				for(j = 0; j < d->qset; j++){
					if(qs->data[j])
						len += sprintf(buf+len, 
							"   % 4i: %8p\n",
							j, qs->data[j]);
				}
			}
		}
		up(&scull_devices[i].sem);
	}
	*eof = 1;
	return len;
}
