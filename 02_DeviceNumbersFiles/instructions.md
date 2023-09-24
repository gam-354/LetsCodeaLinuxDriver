# 02 - Device Numbers and Files

After writing **dev_nr.c** file, build the module by executing

```
make all
```

and then load it into the kernel with

```
sudo insmod dev_nr.ko
```

You can check the log traces in `/var/log/syslog` and you can see "my_dev_nr" in the list of character devices in `/proc/devices`

To link a device to a file, execute
```
sudo mknod <file> c/b MAJOR MINOR
```

which, in this case, would be

```
sudo mknod /dev/mydevice c 91 0
```

(`c` is for character device; `b` for block device)

The /dev/mydevice is now shown with its device numbers associations:

```
$> ls /dev/mydevice -al
crw-r--r-- 1 root root 91, 0 sep 24 17:59 /dev/mydevice
```

## Write a simple program to open and close the device file

Create a file called **testDevice.c** and open the `/dev/mydevice` file as Read-Only. Log the action and close the file afterwards.

Build the program using GCC:

```
gcc testDevice.c -o testDevice
```

Before trying to run the program, let's grant permissions to use the device file:

```
sudo chmod 666 /dev/mydevice
```

Now test the program:

```
./testDevice
```

You should see the log trace in the terminal, as well as a log trace from the dev_nr module in the syslog.

After this, and just for check the non-happy path, you can remove the module and run testDevice to see how it couldn't open the file. The file stil exists, but the kernel module to handle the link to the device is no longer running.