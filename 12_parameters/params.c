#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("Simple linux kernel module to demonstrate the use of parameters");

// Variables holding the kernel module parameters
static unsigned int gpio_id = 12;
static char * device_name = "testdevice";

// Declare the parameters:
module_param(gpio_id, uint, S_IRUGO);
module_param(device_name, charp, S_IRUGO);

// Add some description for each of them
MODULE_PARM_DESC(gpio_id, "ID of the gpio to use");
MODULE_PARM_DESC(device_name, "Device name to use");

// Parameters are automatically stored in the variables, which can be used from
// the initialization of the module
static int __init myInit(void)
{
   printk("Hello mundo!\n");
   printk("Gpio ID: %u\n", gpio_id);
   printk("Device name: %s\n", device_name);
   return 0;
}

static void __exit myExit(void)
{
   printk("Nos vamos!\n");
   return;
}

module_init(myInit);
module_exit(myExit);
