#include <linux/cdev.h>
#include <linux/module.h>

/* DEBUG */

#undef DEBUG_ON

#ifdef DEBUG_MODE
	/* Enable debug messages */
#ifdef __KERNEL__
#define DEBUG_ON(fmt, args...) printk(KERN_DEBUG "scull: " fmt, ## args)
#else
#define DEBUG_ON(fmt, args...) fprintf(stderr, fmt, ## args)
#endif
#else
#define DEBUG_ON(fmt, args...) /*No debug if DEBUG_MODE is not defined*/
#endif

#undef DEBUG_OFF
#define DEBUG_OFF(fmt,args...) /* Disable debug */

/* defines the major number of the devices, if 0 the major is dinamically allocated */
#define SCULL_MAJOR 0
/* first minor to be allocated on the device */
#define SCULL_MINOR 0

#define SCULL_NR_DEVS 4

#define SCULL_QUANTUM 4000
#define SCULL_QSET 1000
extern unsigned int scull_major;
extern unsigned int scull_minor;
extern unsigned int scull_nr_devs; /* number of devices */
extern unsigned int scull_quantum;
extern unsigned int scull_qset;
extern struct scull_dev *scull_devices;
/* device number */
/*dev_t dev;*/
 
/* function prototypes of device setup */
/*int create_dev(void);*/

/* function and variables prototypes of file operations */
extern struct file_operations scull_fops;

//int scull_open(struct inode *inode, struct file *filp);
extern struct scull_qset *scull_follow(struct scull_dev *dev,int n);
extern int create_dev(void);

/* this structure identifies each device into the driver */
struct scull_dev {
        struct scull_qset *data; /* Pointer to first quantum set */
        int quantum;             /* the current quantum size */
        int qset;                /* the current array size */
        unsigned long size;      /* amount of data stored here */
        unsigned int access_key; /* used by sculluid and scullpriv */
        struct semaphore sem;    /* mutual exclusion semaphore */
        struct cdev cdev;        /* Char device structure */
};

/* Member of linked list to keep track of device's data*/
struct scull_qset {
	void **data; /* Stores an array to quantum pointers */
	struct scull_qset *next; /* Next list member */
};


/* proc file function prototypes */
int scull_read_procmem(char *buf, char **start, off_t offset,
                int count, int *eof, void *data);

