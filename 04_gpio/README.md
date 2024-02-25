# 04 - GPIO control through device files

Copy directory of exercise No.3 as we are going to use it as a base. 

Rename `read_write.c` as `gpio.c`, and propagate this change in the Makefile, updating the value to be added to obj-m variable. You may also want to update all the "read_write" references inside the log traces.

In `read_write.c`, delete the declaration of the byte buffer. Update DRIVER_NAME constant according to the current driver:

```
#define DRIVER_NAME "my_gpio_driver"
```

### Include Gpio library and define Gpio ID constants

To work with Gpios, we must include the gpio.h library:

```
#include <linux/gpio.h>
```

The following is just to avoid magic numbers and ease testing different Gpio pins:

```
#define INPUT_GPIO_ID 17
#define OUTPUT_GPIO_ID 4
```

### Expand init and exit functions with Gpio configuration

After adding the device file successfully, the initialization routine should request control for each of the Gpios to be used and configure them.

#### Output pin

Call `gpio_request()` function with the Gpio ID and a name as parameters:

```
   if(gpio_request(OUTPUT_GPIO_ID, "rpi-gpio-4"))
   {
      printk("Can not allocate GPIO 4\n");
      goto AddError;
   }
```

Now, configure the Gpio as output and give it an initial value:

```
   if(gpio_direction_output(OUTPUT_GPIO_ID, 0))
   {
      printk("Can not set GPIO 4 to output\n");
      goto GpioOutError;
   }
```

`GpioOutError` is the label for the de-configuration step to be followed if the direction set operation fails. Just add it above the rest of error handling calls:

```
GpioOutError:
   gpio_free(OUTPUT_GPIO_ID);
```

`gpio_free()` call just drops control of the Gpio ID passed as parameter. 

As part of a normal uninstall of this module, before calling `gpio_free()`, `__myExit()` function may also set a "final" value for the Gpio, before calling the rest of the de-configuration functions:

```
static void __exit myExit(void)
{
   // Undo the steps done in myInit, in reverse order:
   gpio_set_value(OUTPUT_GPIO_ID,0);
   gpio_free(OUTPUT_GPIO_ID);
   (...)
```

#### Input pin

Follow the same procedure as for the Output pin, with the exception that the input pin configuration function does not accept a second parameter indicating the initial value, as it does not make sense.

```
   // INPUT Gpio init:
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
```

GpioInError should call `gpio_free()` with the input Gpio ID. Add this call also in `__myExit()` function.

### Expand driver_read and driver_write functionality:

#### Driver write:

`driver_write` function will be used to apply the value passed from the user side into the Gpio. For that, it must call the `gpio_set_value` function in the library with either a 0 or 1 as arguments, depending on the value requested by the user. 

A byte containing the character entered by the user can be declared and passed to the `copy_from_user()` kernel function

```
   char gpio_value;

   // Copy the data from the user
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
```

The returning value of this function is not as useful as in previous chapters, as gpio value setting functions don't return a result.

#### Driver read:

In `driver_read()` function, we will call gpio_get_value() to get the digital value, then cast it to an ASCII character, and then return it to the user space. We can use a short auxiliary char string to hold the value:

```
   char tmp[3] = " \n";

   to_copy = sizeof(tmp);

   // Read actual value from the HW:
   // (easy way to convert the int to the ASCII value)
   gpio_value = gpio_get_value(INPUT_GPIO_ID);
   tmp[0] = gpio_value + '0';
   printk("Value of input gpio: %d\n", gpio_value);

   // Copy the data to the user
   not_copied = copy_to_user(user_buffer, &tmp, to_copy);
```


#

## Test

After building with `make`, the module is ready to be loaded into the kernel:

```
sudo insmod gpio.ko
```

Please, note that this module will only work fine in hardware whose kernel accepts Gpio management. This example is intended to run in Raspberry Pi boards, where Gpio pins are external and safe to use. Other motherboards may have Gpios linked to specific HW and therefore using them may damage such devices.

When the module is removed, the class and device files are removed, too:

```
sudo rmmod gpio
```
