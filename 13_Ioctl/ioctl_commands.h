#ifndef IOCTL_TEST_H
#define IOCTL_TEST_H

struct myStruct
{
    int repeat;
    char name[64];
};

// The kernel will generate a unique number for every command

#define WR_VALUE _IOW('a', 'b', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)
#define GREETER  _IOW('a', 'c', struct mystruct *)

#endif