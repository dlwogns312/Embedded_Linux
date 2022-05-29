#include <stdio.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <asm/ioctl.h>
#include <fcntl.h>
#include <string.h>

#define SW_NAME "/dev/stopwatch"
#define SW_MAJOR 242