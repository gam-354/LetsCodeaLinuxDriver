# 13 - ioctl

Copy files from exercise **02 - Device numbers**. Rename `dev_nr.c` to `ioctl_example.c` and replace all the references to `dev_nr` to `ioctl_example`, including those in `printk` calls and `Makefile`.

First, let's create a header file to define the command interface between user space and kernel space. Those are the commands that will be passed as arguments of the ioctl calls.

### Commands definition

Create `ioctl_commands.h` file and write in it the following:

```
#ifndef IOCTL_TEST_H
#define IOCTL_TEST_H

struct myStruct
{
    int repeat;
    char name[64];
};

#define WR_VALUE _IOW('a', 'b', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)
#define GREETER  _IOW('a', 'c', struct mystruct *)

#endif
```

We have defined:
* A "Write" command that passes an int32_t value to the kernel module
* A "Read" command that reads an int32_t value from the kernel module
* A simple "Greeter" command that passes a struct to the kernel. The `myStruct` definition is written above.

The kernel will generate a unique "magic number" for each of the command definitions.


### Implement ioctl function

Go back to `ioctl_example.c` and include the following files:

* `<linux/ioctl.h>` for being able to define an ioctl function
* `<linux/uaccess.h>` for using `copy_to_user()` and `copy_from_user()` functions
* `"ioctl_commands.h"` to use our recently defined command identifiers

Define the `my_ioctl()` function:

```
static long int my_ioctl(struct file * file, unsigned cmd, unsigned long arg)
{
    ...
```

A global variable (called `answer` in this example¨) must be defined before to hold the value passed from/to the user space:

```
int32_t answer = 42;
```

Now the function has to detect the command issued from user space and do whatever is needed in each case. Let's use the `switch` statement for it.

* When `WR_VALUE` is called, the module needs to copy the argument from the user space argument to . Therefore, `copy_from_user()` needs to be called. Add some logging to track the results

    ```
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
    ```

+ When `RD_VALUE` is called, the module needs to write the data into the argument, this time serving as a buffer. Therefore `copy_to_user()` must be called this time.

    ```
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
    ```

* When `GREETER` command is called, the argument needs to be copied first from user (casting the pointer to our struct) and then we can access its fields:

    ```
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
    ```

After the closure of the `switch` statement, the function must return a value to indicate the result of the overall operation. Let's reduce it to returning `0`.

### Update file operations struct:

Indicate the kernel that this module is able to serve ioctl calls by adding the .unlocked_ioctl field pointing to the function we have just defined:

```
static struct file_operations fops = {
    (...)
   .unlocked_ioctl = my_ioctl
};
```

## Modify test application

Now go to `testDevice.c` file (now renamed simply as `test.c`) and include two new files: one is `sys/ioctl.h`, to make the ioctl calls, and the other one is our definitions file `ioctl_commands.h`.

Declare an `int` variable and a `myStruct` object. Those will be the variables involved in our `ioctl` calls.

```
int answer;
struct myStruct test = {3, "Pepe"};
```

In the `main()` function, modify the open() call so that the permissions argument is now write and read only: `O_WRONLY`.

After the successful initialization, add some `ioctl` calls. One for reading the initial value of the integer variable from the kernel module:

    ioctl(dev, RD_VALUE, &answer);
    printf("The answer is %d\n", answer);

Then another one for writing a new value and checking it afterwards:

    answer = 123;
    ioctl(dev, WR_VALUE, &answer);
    
    // Read it back to check
    ioctl(dev, RD_VALUE, &answer);
    printf("The answer now is %d\n", answer);

And finally, an `ioctl` call using `GREETER` command, passing the address of the test structure:

    ioctl(dev, GREETER, &test);

You can now build both the kernel module (using `make` command) and the `test` program (using `gcc`).

## Check the results

Load the kernel module from your terminal using `insmod` command. You can check the log traces in `/var/log/syslog` showing the Major and Minor numbers assigned for our device. To link a device to a file, execute
```
sudo mknod <file> c/b MAJOR MINOR
```
which, in this case, would be

```
sudo mknod /dev/dummy c 91 0
```
(`c` is for character device; `b` for block device)

Remember that the name of this file must match the name written in the test application. Also remember to give `rwx` permissions to the file so that the test app can open it.

You should see the `printf` traces in the terminal:

```
$> ./test
The answer is 42
The answer now is 123
Opening successful!
```
and the kernel traces in the `syslog` file as well:

```
[  820.314747] ioctl_example - Hello mundo!
[  820.314749] ioctl_example - registerd Device number Major: 91, Minor, 0
[ 1086.974333] ioctl_example - open was called!
[ 1086.974337] ioctl_example - the answer was copied
[ 1086.974360] ioctl_example - update the answer to 123
[ 1086.974361] ioctl_example - the answer was copied
[ 1086.974363] ioctl_example - 3 greets to Pepe
[ 1086.974371] ioctl_example - close was called!
```



