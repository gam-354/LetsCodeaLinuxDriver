#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("Registers a device number and implements some callback functions");

// Buffer for data
static char buffer[255];
static int buffer_pointer = 0;

// Variables for device and device class
static dev_t my_device_nr;       // Will hold the device number assigned by the kernel
static struct class *my_class;
static struct cdev my_device; 

#define DRIVER_NAME "dummydriver"
#define DRIVER_CLASS "MyModuleClass5"

/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file * File, char * user_buffer, size_t count, loff_t * offset)
{
   int to_copy, not_copied, delta;

   // 1. Get the amount of data to copy, which will be the minimum
   // between the amount of bytes requested and the amount of bytes
   // stored in the buffer (indicated by the pointer value)
   to_copy = min(count, buffer_pointer);

   // 2. Copy the data to the user
   not_copied = copy_to_user(user_buffer, buffer, to_copy);

   // 3. Calculate how much data it has copied
   delta = to_copy - not_copied;
   return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file * File, const char * user_buffer, size_t count, loff_t * offset)
{
   int to_copy, not_copied;

   // 1. Get the amount of data to copy, which will be the minimum
   // between the amount of bytes requested and the size of the buffer
   to_copy = min(count, sizeof(buffer));

   // 2. Copy the data to the user
   not_copied = copy_from_user(buffer, user_buffer, to_copy);
   
   // 3. Update the pointer, whose value also indicates how much
   // data has been written
   buffer_pointer = to_copy - not_copied;
   return buffer_pointer;
}

/**
 * @brief function called when the device file is opened
 */
static int driver_open(struct inode * device_file, struct file * instance) 
{
   printk("read_write - open was called!\n");
   return 0;
}

/**
 * @brief function called when the device file is closed
 */
static int driver_close(struct inode * device_file, struct file * instance) 
{
   printk("read_write - close was called!\n");
   return 0;
}

static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = driver_open,
   .release = driver_close,
   .read = driver_read,
   .write = driver_write
};

#define MY_MAJOR 91     // Free device number. Check list in cat /proc/devices


/**
 * @brief function called when the module is loaded into the kernel
 */

static int __init myInit(void)
{
   printk("read_write - Hello mundo!\n");

   // 1. Allocate a device nr.
   // The function will write the major and minor numbers in my_device_nr.
   
   if (alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0)
   {
      printk("Device Nr. could not be allocated!\n");
      return -1;
   }

   int major = my_device_nr >> 20;
   int minor = my_device_nr & 0xFFFFF;
   printk("read_write - Device Nr. Major: %d, Minor: %d, was registered\n", major, minor);
   
   // 2. Create device class
   if((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL)
   {
      printk("Device class can not be created\n");
      goto ClassError;
   }

   // 3. Create device file
   if(device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL)
   {
      printk("Can not create device file\n");
      goto FileError;
   }

   // 4. Initialize device file
   cdev_init(&my_device, &fops);

   // 5. Add the device file
   if(cdev_add(&my_device, my_device_nr, 1) == -1)
   {
      printk("Registering of device to kernel failed!\n");
      goto AddError;
   }

   return 0;

   // Error cases are managed with "goto" instructions so
   // that it is easy to undo all steps done so far at the 
   // moment of the error 

AddError:
   device_destroy(my_class, my_device_nr);
FileError:
   class_destroy(my_class);
ClassError:
   unregister_chrdev_region(my_device_nr, 1);
   return -1;

}

/**
 * @brief function called when the module is unloaded from the kernel
 */
static void __exit myExit(void)
{
   // Undo the steps done in myInit, in reverse order:
   cdev_del(&my_device);
   device_destroy(my_class, my_device_nr);
   class_destroy(my_class);
   unregister_chrdev_region(my_device_nr, 1);
   printk("read_write - bye bye!\n");
   return;
}

module_init(myInit);
module_exit(myExit);





