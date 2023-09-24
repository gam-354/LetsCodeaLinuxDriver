#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>	

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("Registers a device number and implements some callback functions");

/**
 * @brief function called when the device file is opened
 */
static int driver_open(struct inode * device_file, struct file * instance) 
{
   printk("dev_nr - open was called!\n");
   return 0;
}

/**
 * @brief function called when the device file is closed
 */
static int driver_close(struct inode * device_file, struct file * instance) 
{
   printk("dev_nr - close was called!\n");
   return 0;
}

static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = driver_open,
   .release = driver_close
};

#define MY_MAJOR 91     // Free device number. Check list in cat /proc/devices


/**
 * @brief function called when the module is loaded into the kernel
 */

static int __init myInit(void)
{
   int retVal;
   printk("dev_nr - Hello mundo!\n");

   // Register the device number for a new character device
   retVal = register_chrdev(MY_MAJOR, "my_dev_nr", &fops);

   // retval contains some info encoded.

   // If 0, this means that the slot was free and this is the first device registered in it.
   if(retVal == 0)
   {
      printk("dev_nr - registerd Device number Major: %d, Minor, %d\n", MY_MAJOR, 0);
   }
   // If positive, there is already one device registered. Check Major and Minor returned values
   // by shifting some bits:
   else if(retVal > 0)
   {
      int major = retVal >> 20;
      int minor = retVal & 0xFFFFF;
      printk("dev_nr - registerd Device number Major: %d, Minor, %d\n", major, minor);
   }
   // If negative, then something went wrong
   else
   {
      printk("dev_nr - Could not register device number!\n");
      return -1;
   }
   return 0;
}

/**
 * @brief function called when the module is loaded into the kernel
 */
static void __exit myExit(void)
{
   // Unregister our device
   unregister_chrdev(MY_MAJOR, "my_dev_nr");
   printk("dev_nr - Nos vamos!\n");
   return;
}

module_init(myInit);
module_exit(myExit);





