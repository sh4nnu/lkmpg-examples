/*
 * chardev.h - the header file with the ioctl definitions.
 *
 * The declarations are in a header file, because 
 * both kernel module and process calling it should need.
 *
 */

#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h>

#define MAJOR_NUM 100

#define IOCTL_SET_MSG _IOW(MAJOR_NUM, 0, char *)

#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char *)

#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, int)

#define DEVICE_FILE_NAME "char_dev"

#endif

