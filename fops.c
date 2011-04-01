#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include "scull.h"

/* File operations in use by the scull driver */

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



struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.llseek = scull_llseek,
	.read = scull_read,
	.write = scull_write,
	.ioctl = scull_ioctl,
	.open = scull_open,
	.release = scull_release,
};
