#include <linux/module.h>
#include <linux/init.h>

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/ioctl.h>

#include <linux/poll.h>
#include <linux/wait.h>

#include "defs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("A simple example for sending poll from a LKM to user space");


static int irq_ready = 0;
static wait_queue_head_t waitqueue;

#define MY_MAJOR 91     // Free device number. Check list in cat /proc/devices

// Poll callback:
static unsigned int my_poll(struct file * file, poll_table * wait)
{
   poll_wait(file, &waitqueue, wait);  // Won't take CPU resources while waiting
   if (irq_ready == 1)
   {
      irq_ready = 0;
      return POLLIN;
   }
   return 0;
}

// IOCTL function for unocking the UserSpace app from the wait induced by polling
static long int my_ioctl(struct file * file, unsigned cmd, unsigned long arg) 
{
   if (cmd == CMD_UNLOCK)
   {
      printk("poll - Waking up polling processes\n");
      irq_ready = 1;
      wake_up(&waitqueue);
   }
   return 0;
}

/**
 * @brief function called when the device file is closed
 */
static int my_close(struct inode * device_file, struct file * instance) 
{
   printk("poll - close was called!\n");
   return 0;
}

static struct file_operations fops = {
   .owner = THIS_MODULE,
   .unlocked_ioctl = my_ioctl,    // name of ioctl function
   .poll = my_poll,
   .release = my_close
};

static int __init myInit(void)
{
   // Init waitqueue
   init_waitqueue_head(&waitqueue);
   
   // Register the device number for a new character device
   int retVal = register_chrdev(MY_MAJOR, "LKM_poll", &fops);

   // If 0, this means that the slot was free and this is the first device registered in it.
   if(retVal == 0)
   {
      printk("poll - registered Device number Major: %d, Minor, %d\n", MY_MAJOR, 0);
   }
   // If positive, there is already one device registered. Check Major and Minor returned values
   // by shifting some bits:
   else if(retVal > 0)
   {
      int major = retVal >> 20;
      int minor = retVal & 0xFFFFF;
      printk("poll - registered Device number Major: %d, Minor, %d\n", major, minor);
   }
   // If negative, then something went wrong
   else
   {
      printk("poll - Could not register device number!\n");
      return -1;
   }

   return 0;
}

static void __exit myExit(void)
{
   printk("poll - exiting!\n");
   unregister_chrdev(MY_MAJOR, "LKM_poll");
   return;
}

module_init(myInit);
module_exit(myExit);
