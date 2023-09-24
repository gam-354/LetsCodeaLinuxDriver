#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("El veloz murcielago...");

static int __init myInit(void)
{
   printk("Hello mundo!\n");
   return 0;
}

static void __exit myExit(void)
{
   printk("Nos vamos!\n");
   return;
}

module_init(myInit);
module_exit(myExit);
