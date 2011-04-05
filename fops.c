#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include "scull.h"

/* File operations in use by the scull driver */

int scull_trim(struct scull_dev *dev)
{
	struct scull_qset *next, *dptr;
	int qset = dev->qset; /*dev is not null*/
	int i;

	for(dptr = dev->data; dptr; dptr = next){ /* All the list items*/
		if(dptr->data){
			for(i=0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next=dptr->next;
		kfree(dptr);
	}
	dev->size = 0;
	dev->quantum = scull_quantum;
	dev->qset = scull_qset;
	dev->data = NULL;
	return 0;
}

int scull_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev; /* Device information */
	
	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev; /* for other methods */

	/* now, trim to 0 the length of the device if open was write-only*/
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY){
		scull_trim(dev);
	}
	return 0; /*success*/
}

/* Nothing special to do, since we don't have any hardware to shutdown for while */
int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr; /* the fisrt list item */
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset; /* how many bytes in the list item */
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if(*f_pos >= dev->size)
		goto out;
	if(*f_pos + count > dev->size)
		count = dev->size - *f_pos;
	
	/* find listitem, qset index and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	/* follow the list up to the right position (defined elsewhere) */
	dptr = scull_follow(dev,item);

	if(dptr == NULL || !dptr->data || !dptr->data[s_pos])
		goto out; /* don't fill holes */

	/*read only up to the end of this quantum */
	if(count > quantum - q_pos)
		count = quantum - q_pos;

	if(copy_to_user(buf, dptr->data[s_pos] + q_pos,count)){
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;

	out:
		up(&dev->sem);
		return retval;
}	
		
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr; /*first list item */
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* find listitem qset index and offset in the quamtum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = scull_follow(dev,item);
	if(dptr == NULL)
		goto out;

	if(!dptr->data){
		dptr->data = kmalloc(qset * sizeof(char*), GFP_KERNEL);
		
		if(!dptr->data)
			goto out;
	
		memset(dptr->data,0,qset * sizeof (char*));
	}

	if(!dptr->data[s_pos]){
		dptr->data[s_pos] = kmalloc(quantum,GFP_KERNEL);

		if(!dptr->data[s_pos])
			goto out;
	}

	/* write only up to the end of this quantum */
	if(count > quantum - q_pos)
		count = quantum - q_pos;

	if(copy_from_user(dptr->data[s_pos]+q_pos, buf, count)){
		retval = -EFAULT;
		goto out;
	}
	
	*f_pos += count;
	retval = count;

	/* update the size */
	if(dev->size < *f_pos)
		dev->size = *f_pos;

	out:
		up(&dev->sem);
		return  retval;
}
			
struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	/*.llseek = scull_llseek,*/
	.read = scull_read,
	.write = scull_write,
	/*.ioctl = scull_ioctl,*/
	.open = scull_open,
	.release = scull_release,
};
