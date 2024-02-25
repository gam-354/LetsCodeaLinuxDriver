#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("A simple gpio driver");


// Variables for device and device class
static dev_t my_device_nr;       // The device number assigned by the kernel
static struct class *my_class;   // Pointer to the driver class 
static struct cdev my_device;    // The device object

#define DRIVER_NAME "my_gpio_driver"
#define DRIVER_CLASS "MyModuleClass"

#define INPUT_GPIO_ID 17
#define OUTPUT_GPIO_ID 4

/**
 * @brief Read data. Used to read the INPUT Gpio value as text
 */
static ssize_t driver_read(struct file * File, char * user_buffer, size_t count, loff_t * offset)
{
   int to_copy, not_copied, gpio_value;
   char tmp[3] = " \n";

   // 1. Get the amount of data to copy
   to_copy = sizeof(tmp);

   // 2. Read actual value from the HW:
   // (easy way to convert the int to the ASCII value)
   gpio_value = gpio_get_value(INPUT_GPIO_ID);
   tmp[0] = gpio_value + '0';
   printk("Value of input gpio: %d\n", gpio_value);

   // 3. Copy the data to the user
   not_copied = copy_to_user(user_buffer, &tmp, to_copy);

   // 4. Return the amount of bytes read
   return 1;
}

/**
 * @brief Write data. Used to set the OUTPUT Gpio value
 */
static ssize_t driver_write(struct file * File, const char * user_buffer, size_t count, loff_t * offset)
{
   int to_copy, not_copied;
   char gpio_value;

   // 1. Get the amount of data to copy
   to_copy = sizeof(gpio_value);

   // 2. Copy the data from the user
   not_copied = copy_from_user(&gpio_value, user_buffer, to_copy);

   switch(gpio_value)
   {
      case '0':
         gpio_set_value(OUTPUT_GPIO_ID,0);
         break;
      case '1':
         gpio_set_value(OUTPUT_GPIO_ID,1);
         break;
      default:
         printk("Invalid output value to be set\n");
         break;
   }
   
   // 3. Return the amount of bytes written
   return 1;
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

   // 6. OUTPUT Gpio init
   if(gpio_request(OUTPUT_GPIO_ID, "rpi-gpio-4"))
   {
      printk("Can not allocate GPIO 4\n");
      goto AddError;
   }

   // Set GPIO direction as OUTPUT and 0 as the initial value
   if(gpio_direction_output(OUTPUT_GPIO_ID, 0))
   {
      printk("Can not set GPIO 4 to output\n");
      goto GpioOutError;
   }

   // 7. INPUT Gpio init:
   if(gpio_request(INPUT_GPIO_ID, "rpi-gpio-17"))
   {
      printk("Can not allocate input GPIO ID\n");
      goto GpioOutError;
   }

   // Set GPIO direction as INPUT
   if(gpio_direction_input(INPUT_GPIO_ID))
   {
      printk("Can not set GPIO To input\n");
      goto GpioInError;
   }

   return 0;

   // Error cases are managed with "goto" instructions so
   // that it is easy to undo all steps done so far at the 
   // moment of the error 
GpioInError:
   gpio_free(INPUT_GPIO_ID);
GpioOutError:
   gpio_free(OUTPUT_GPIO_ID);
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
   gpio_free(INPUT_GPIO_ID);
   gpio_set_value(OUTPUT_GPIO_ID,0);
   gpio_free(OUTPUT_GPIO_ID);
   cdev_del(&my_device);
   device_destroy(my_class, my_device_nr);
   class_destroy(my_class);
   unregister_chrdev_region(my_device_nr, 1);
   printk("read_write - bye bye!\n");
   return;
}

module_init(myInit);
module_exit(myExit);





