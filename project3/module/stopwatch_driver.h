#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/workqueue.h?

//define the address of FND
#define FND_ADDRESS 0x08000004

//define name and major number
#define SW_NAME "stopwatch"
#define SW_MAJOR 242

static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

//handler function
irqreturn_t inter_home(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_back(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_volume_up(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_volume_down(int irq, void* dev_id, struct pt_regs* reg); 