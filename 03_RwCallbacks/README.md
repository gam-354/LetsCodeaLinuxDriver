# 03 - Auto Device File creation & Read/Write callbacks

Copy directory of exercise No.2 as we are going to use it as a base. You can remove the C program to test the module.

Rename `dev_nr.c` as `read_write.c`, and propagate this change in the Makefile, updating the value to be added to obj-m variable. You may also want to update all the "dev_nr" references inside the log traces

### Define an internal buffer

Declare the byte buffer and an integer to control how much size of the buffer is used

```
#define DRIVER_NAME "dummydriver"
#define DRIVER_CLASS "MyModuleClass"
```

### Define read and write functions

Write `driver_read` and `driver_write` functions, which basically call `copy_to_user` and `copy_from_user` kernel functions, respectively.

```
static ssize_t driver_read(struct file * File, char * user_buffer, size_t count, loff_t * offset)
static ssize_t driver_write(struct file * File, const char * user_buffer, size_t count, loff_t * offset)
```

### Update file operations struct

Link `.read` and `.write` operatios to the functions just defined.

```
static struct file_operations fops = {
   (...),
   .read = driver_read,
   .write = driver_write
};
```

### Declare some variables for device and device class

```
static dev_t my_device_nr;       // The device number assigned by the kernel
static struct class *my_class;   // Pointer to the driver class 
static struct cdev my_device;    // The device object
```

### Write actions to have the device file automatically on module load

`myInit` function is the place where the module should create and initialize the device file. Some code must be added:

1. Request a device number for our driver:

   ```
   if (alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0)
   {
      printk("Device Nr. could not be allocated!\n");
      return -1;
   }

   int major = my_device_nr >> 20;
   int minor = my_device_nr & 0xFFFFF;
   printk("read_write - Device Nr. Major: %d, Minor: %d, was registered\n", major, minor);
   ```

2. Initialize the device class, by calling class_create() and passing our driver module ID and a driver class name, which has been arbitrarily set before:

   ```
   if((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL)
   {
      printk("Device class can not be created\n");
      goto ClassError;
   }
   ```

    Here is an example of `goto` instruction. We will define a bunch of error management actions after all the steps. This approach makes sense, as we will see below.

3. Create a device file object, by calling device_create(), passing the device class pointer, the device number obtained and our driver name:

   ```
   if(device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL)
   {
      printk("Can not create device file\n");
      goto FileError;
   }
   ```

4. Initialize the device file, indicating the file operations allowed by this device:

   ```
   cdev_init(&my_device, &fops);
   ```

5. Add the device file to the list:

   ```
   if(cdev_add(&my_device, my_device_nr, 1) == -1)
   {
      printk("Registering of device to kernel failed!\n");
      goto AddError;
   }
   ```

6. **Error management**: if the last function call returns with success, then there is nothing more to do:

    ```
   return 0;
    ```

    If any of the steps above fails, the code will jump into the following section at one of its levels:

   ```
   AddError:
      device_destroy(my_class, my_device_nr);
   FileError:
      class_destroy(my_class);
   ClassError:
      unregister_chrdev_region(my_device_nr, 1);
      return -1;
   ```

   This design allows reversing as much actions as done so far, because the program will run from one of the three entry points until the `return` statement. For example, an error in step 3 (`device_create`) will make the program jump into FileError section and will destroy the class (undo step 2), then call `unregister_chrdev_region()` (undo step 1) and finally return -1.

## Test

After building with `make`, the module is ready to be loaded into the kernel:

```
sudo insmod read_write.ko
```

Apart from some traces in the syslog, the device can be seen in /proc/devices:

```
$> cat /proc/devices
Character devices:
(...)
510 dummydriver
```

The **510** number may be different depending on the system. It is just the number that the kernel decided to assign to this driver. This can be confirmed with the syslog traces:

```
[ 1659.465667] read_write - Device Nr. Major: 510, Minor: 0, was registered

```

But the most interesting feature of this module is that the device file is now ready to use. The list of device files (`/dev`) should show `dummydriver`. Before using it, we must provide it with R/W permissions:

```
sudo chmod 666 /dev/dummydriver
```

Also, note that the device class file has also been created in `/sys/class/MyModuleClass`.

You can now write some bytes to the driver with the `echo` command:

```
echo "hello, driver" > /dev/dummydriver
```

which can be recovered by reading from the same files with, for example, head command:

```
$> head -n 1 /dev/dummydriver
hello, driver
```

Note that further write operations on the device file will override the previous contents, as we have not provided our code with the functionality to append bytes to the end of the buffer.

When the module is removed, the class and device files are removed, too:

```
sudo rmmod read_write
```
