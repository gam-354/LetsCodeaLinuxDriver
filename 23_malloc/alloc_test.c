#include <linux/module.h>
#include <linux/init.h>

#include <linux/slab.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("Example with dynamic memory management");

struct driver_data {
   u8 version;
   char text[64];
};

u32 * ptr1;
struct driver_data * ptr2;

static int __init myInit(void)
{
   // Kmalloc: just memory allocation

   ptr1 = kmalloc(sizeof(u32), GFP_KERNEL);

   if(ptr1 == NULL)
   {
      printk("alloc_test - out of memory!\n");
      return -1;
   }
   printk("alloc_test - [kmalloc][Before assigning] - *ptr1: 0x%x \n", *ptr1);
   *ptr1 = 0x0A0B0C0D;
   printk("alloc_test - [kmalloc][After assigning] - *ptr1: 0x%x\n", *ptr1);
   kfree(ptr1);

   // Kzalloc: memory allocation and initialization to zero
   ptr1 = kzalloc(sizeof(u32), GFP_KERNEL);

   if(ptr1 == NULL)
   {
      printk("alloc_test - out of memory!\n");
      return -1;
   }
   printk("alloc_test - [kzalloc][Before assigning] - *ptr1: 0x%x\n", *ptr1);
   *ptr1 = 0x0A0B0C0D;
   printk("alloc_test - [kzalloc][After assigning] - *ptr1: 0x%x\n", *ptr1);
   kfree(ptr1);

   // Kzalloc: example with the struct object
   ptr2 = kzalloc(sizeof(struct driver_data), GFP_KERNEL);

   if(ptr2 == NULL)
   {
      printk("alloc_test - out of memory!\n");
      return -1;
   }

   printk("alloc_test - [kzalloc][Before assigning] - ptr2->version: %u \n", ptr2->version);
   printk("alloc_test - [kzalloc][Before assigning] - ptr2->text: %s \n", ptr2->text);

   ptr2->version = 123;
   strcpy(ptr2->text, "This is a test string for my Linux Kernel Module");

   printk("alloc_test - [kzalloc][After assigning] - ptr2->version: %u \n", ptr2->version);
   printk("alloc_test - [kzalloc][After assigning] - ptr2->text: %s \n", ptr2->text);

   return 0;
}

static void __exit myExit(void)
{
   // Memory will persist until manually de-allocated
   printk("alloc_test - [kzalloc][On module exit] - ptr2->version: %u \n", ptr2->version);
   printk("alloc_test - [kzalloc][On module exit] - ptr2->text: %s \n", ptr2->text);
   kfree(ptr2);
   return;
}

module_init(myInit);
module_exit(myExit);
