#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>

#include "ioctl_commands.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("A simple example for ioctl in a LKM");

/**
 * @brief function called when the device file is opened
 */
static int driver_open(struct inode * device_file, struct file * instance) 
{
   printk("ioctl_example - open was called!\n");
   return 0;
}

/**
 * @brief function called when the device file is closed
 */
static int driver_close(struct inode * device_file, struct file * instance) 
{
   printk("ioctl_example - close was called!\n");
   return 0;
}

// Global variable for reading and writing:
int32_t answer = 42;


// Standard definition of a ioctl call: file pointer, command and arg(s)
static long int my_ioctl(struct file * file, unsigned cmd, unsigned long arg)
{
   struct myStruct test;

   switch(cmd)
   {
      case WR_VALUE:
         if(copy_from_user(&answer, (int32_t *) arg, sizeof(answer)))
         {
            printk("ioctl_example - Error copying data from user!\n");
         }
         else
         {
            printk("ioctl_example - update the answer to %d\n", answer);
         }
         break;

      case RD_VALUE:
         if(copy_to_user((int32_t *) arg, &answer, sizeof(answer)))
         {
            printk("ioctl_example - Error copying data to user!\n");
         }
         else
         {
            printk("ioctl_example - the answer was copied\n");
         }
         break;

      case GREETER:
         if(copy_from_user(&test, (struct myStruct *) arg, sizeof(test)))
         {
            printk("ioctl_example - Error copying data from user!\n");
         }
         else
         {
            printk("ioctl_example - %d greets to %s\n", test.repeat, test.name);
         }
         break;
   }

   return 0;
}


static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = driver_open,
   .release = driver_close,
   .unlocked_ioctl = my_ioctl    // Add name of ioctl function
};

#define MY_MAJOR 91     // Free device number. Check list in cat /proc/devices


/**
 * @brief function called when the module is loaded into the kernel
 */

static int __init myInit(void)
{
   int retVal;
   printk("ioctl_example - Hello mundo!\n");

   // Register the device number for a new character device
   retVal = register_chrdev(MY_MAJOR, "my_ioctl_example", &fops);

   // retval contains some info encoded.

   // If 0, this means that the slot was free and this is the first device registered in it.
   if(retVal == 0)
   {
      printk("ioctl_example - registerd Device number Major: %d, Minor, %d\n", MY_MAJOR, 0);
   }
   // If positive, there is already one device registered. Check Major and Minor returned values
   // by shifting some bits:
   else if(retVal > 0)
   {
      int major = retVal >> 20;
      int minor = retVal & 0xFFFFF;
      printk("ioctl_example - registerd Device number Major: %d, Minor, %d\n", major, minor);
   }
   // If negative, then something went wrong
   else
   {
      printk("ioctl_example - Could not register device number!\n");
      return -1;
   }
   return 0;
}

/**
 * @brief function called when the module is unloaded from the kernel
 */
static void __exit myExit(void)
{
   // Unregister our device
   unregister_chrdev(MY_MAJOR, "my_ioctl_example");
   printk("ioctl_example - Nos vamos!\n");
   return;
}

module_init(myInit);
module_exit(myExit);





