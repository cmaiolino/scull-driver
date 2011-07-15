#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/ioctl.h>

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

/*int scull_open(struct inode *inode, struct file *filp);*/
extern struct scull_qset *scull_follow(struct scull_dev *dev,int n);
extern int create_dev(void);
extern void procfile_setup(void);

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

/* IOCTLs */

/*sets the scull magic number*/
#define SCULL_IOC_MAGIC 0xF7 

#define SCULL_IOCRESET _IO(SCULL_IOC_MAGIC, 0)

/*
 * S means 'Set' through a ptr
 * T means "Tell" directly with the argument value
 * G means "GET": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 */

#define SCULL_IOCSQUANTUM 	_IOW(SCULL_IOC_MAGIC, 1, int)
#define SCULL_IOCSQSET	 	_IOW(SCULL_IOC_MAGIC, 2, int)
#define SCULL_IOCTQUANTUM	_IO(SCULL_IOC_MAGIC, 3)
#define SCULL_IOCTQSET		_IO(SCULL_IOC_MAGIC, 4)
#define SCULL_IOCGQUANTUM	_IOR(SCULL_IOC_MAGIC, 5, int)
#define SCULL_IOCGQSET		_IOR(SCULL_IOC_MAGIC, 6, int)
#define SCULL_IOCQQUANTUM	_IO(SCULL_IOC_MAGIC, 7)
#define SCULL_IOCQQSET		_IO(SCULL_IOC_MAGIC, 8)
#define SCULL_IOCXQUANTUM	_IOWR(SCULL_IOC_MAGIC, 9, int)
#define SCULL_IOCXQSET		_IOWR(SCULL_IOC_MAGIC, 10, int)
#define SCULL_IOCHQUANTUM	_IO(SCULL_IOC_MAGIC, 11)
#define SCLL_IOcHQSET		_IO(SCULL_IOC_MAGIC, 12)

#define SCULL_IOC_MAXNR 14
