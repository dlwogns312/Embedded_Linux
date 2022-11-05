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
#include <linux/delay.h>
#include <linux/timer.h>

#define DEVICE_MAJOR 242
#define DEVICE_NAME "dev_lib"

#define LCD_ADDRESS 0x08000090
#define SWITCH_ADDRESS 0x08000050	

int dev_driver_open(struct inode *minode, struct file* mfile);
int dev_driver_release(struct inode *minode,struct file *mfile);
ssize_t dev_driver_write(struct file *inode, const char* gdata, size_t length, loff_t *off_what);
ssize_t dev_driver_read(struct file *inode,  char* gdata, size_t length, loff_t *off_what);

//irqreturn_t inter_back(int irq,void* dev_id, struct pt_regs*reg);
void display(void);
