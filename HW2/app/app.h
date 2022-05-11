#include <stdio.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <asm/ioctl.h>
#include <fcntl.h>
#include <string.h>

#define ESSW_DEVICE_NAME "/dev/dev_driver"
#define ESSW_DEVICE_MAJOR 242
#define ESSW_DEVICE_SET      _IOW(ESSW_DEVICE_MAJOR, 0, struct data_packet)
#define ESSW_DEVICE_OPERATE  _IOW(ESSW_DEVICE_MAJOR, 1, struct data_packet)


struct data_packet {
    unsigned char interval;
    unsigned char cnt;
    unsigned char init[4];
};