#include <linux/module.h>
#include <linux/init.h>

#include <linux/kobject.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("Creating a folder and a file in sysfs");

// Note: using macro to initialize the attributes easily:
// - Object name
// - Permissions
// - Read/show callback
// - Write/store callback

// Read callback for /sys/hello/dummy:
// 1st arg - The folder
// 2nd arg - The file
static ssize_t dummy_show(struct kobject * kobj, struct kobj_attribute * attr, char * buffer)
{
   // Just write something into the buffer and return the amount of bytes written
   return sprintf(buffer, "You have read from /sys/kernel/%s%s\n",
      kobj->name, attr->attr.name);
}

// Write callback for /sys/hello/dummy:
// 1st arg - The folder
// 2nd arg - The file
static ssize_t dummy_store(struct kobject * kobj, struct kobj_attribute * attr, const char * buffer, size_t count)
{
   // Print in kernel what was passed from the sysfs
   printk("sysfs - You wrote '%s' to /sys/kernel/%s%s\n", buffer, kobj->name, attr->attr.name);
   return count;
}

// Global variable for sysfs folder "hello"
static struct kobject * dummy_kobj;
static struct kobj_attribute dummy_attr = __ATTR(dummy, 0660, dummy_show, dummy_store);


static int __init myInit(void)
{
   printk("sysfs - Creating /sys/kernel/hello/dummy\n");

   // 1. Create the sysfs "hello" folder
   // kernel_kobj is an internal kernel object already initialized
   dummy_kobj = kobject_create_and_add("hello", kernel_kobj);

   if(dummy_kobj == NULL)
   {
      printk("sysfs - Error creating the sysfs 'hello' folder\n");
      return -ENOMEM;
   }

   // 2. Create the "dummy" file in /sys/kernel/hello
   // (function returning 0 on success)
   if(sysfs_create_file(dummy_kobj, &dummy_attr.attr))
   {
      printk("sysfs - Error creating the sysfs 'dummy' file\n");
      return -ENOMEM;
   }
   return 0;
}

static void __exit myExit(void)
{
   printk("sysfs - Deleting file and kobject\n");
   
   sysfs_remove_file(dummy_kobj, &dummy_attr.attr);

   // It would have been enough to remove the kobject
   kobject_put(dummy_kobj);
   return;
}

module_init(myInit);
module_exit(myExit);
