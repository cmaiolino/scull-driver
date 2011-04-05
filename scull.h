#include <linux/cdev.h>
#include <linux/module.h>


/* defines the major number of the devices, if 0 the major is dinamically allocated */
#define SCULL_MAJOR 0
/* first minor to be allocated on the device */
#define SCULL_MINOR 0

#define SCULL_QUANTUM 4000
#define SCULL_QSET 1000
extern unsigned int scull_major;
extern unsigned int scull_minor;
extern unsigned int scull_nr_devs; /* number of devices */
extern unsigned int scull_quantum;
extern unsigned int scull_qset;
extern struct scull_dev *dev;
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



