#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
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

/* seq_file proc methods */

static void *scull_seq_start(struct seq_file *s, loff_t *pos) /* pos is an index to a scull_devices array */
{
	if (*pos >= scull_nr_devs)
		return NULL; /* no more to read */

	return scull_devices + *pos;
}

static void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos) /* *v is the iterator returned by previous call to start or next */
{
	(*pos)++;
	if (*pos >= scull_nr_devs){
		return NULL;
	}
	return scull_devices + *pos;
}

static void scull_seq_stop(struct seq_file *s, void *v)
{
}

static int scull_seq_show(struct seq_file *s, void *v)
{
	struct scull_dev *dev = (struct scull_dev *) v;
	struct scull_qset *d;
	int i;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	seq_printf(s, "\nDevice %i: qset %i, q %i, sz %li\n",
		(int) (dev - scull_devices), dev->qset,
		dev->quantum, dev->size);

	for(d = dev->data; d; d = d->next) { /* scan the list */
		seq_printf(s, "   item at %p, qset at %p\n", d, d->data);
		if(d->data && !d->next) /*dump only the last item */
			for(i=0; i < dev->qset; i++){
				if(d->data[i])
					seq_printf(s, "     % 4i: %8p\n",
						i, d->data[i]);
			}
	}
	up(&dev->sem);
	return 0;
}





static struct seq_operations scull_seq_ops = {
	.start = scull_seq_start,
	.next  = scull_seq_next,
	.show  = scull_seq_show,
	.stop  = scull_seq_stop
};

static int scull_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &scull_seq_ops);
}

static struct file_operations scull_proc_ops = {
	.owner = THIS_MODULE,
	.open  = scull_proc_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};


void procfile_setup(void)
{
	struct proc_dir_entry *entry;

	entry = create_proc_entry("scullseq", 0, NULL);

	if(entry)
		entry->proc_fops = &scull_proc_ops;
}

/* No clean up work to do, no stop method then */
