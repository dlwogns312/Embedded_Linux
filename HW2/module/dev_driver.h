#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/ioctl.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/uaccess.h>

// IOCTL COMMAND and Device Driver Information 
#define ESSW_DEVICE_NAME "dev_driver"
#define ESSW_DEVICE_MAJOR 242
#define ESSW_DEVICE_SET      _IOW(ESSW_DEVICE_MAJOR, 0, struct data_packet)
#define ESSW_DEVICE_OPERATE  _IOW(ESSW_DEVICE_MAJOR, 1, struct data_packet)

// Address of Components in FPGA
#define FPGA_TEXT_LCD_ADDRESS 0x08000090 // pysical address - 32 Byte (16 * 2)
#define FPGA_FND_ADDRESS 0x08000004
#define FPGA_LED_ADDRESS 0x08000016
#define FPGA_DOT_ADDRESS 0x08000210

// For TEXT_LCD
#define ID_LEN 8
#define NAME_LEN 12

// Magic Number Flag for output wrapper function
#define TIMER_DONE 300
#define TIMER_IN_PROCESS 301

#define LEFT 0
#define RIGHT 1

struct data_packet {
    unsigned char interval;
    unsigned char cnt;
    unsigned char init[4];
};

// Define device driver functions in the moudle
int essw_fpga_device_open(struct inode *minode, struct file *mfile);
int essw_fpga_device_release(struct inode *minode, struct file *mfile);
// int essw_fpga_device_ioctl (struct inode *minode, struct file *mfile, unsigned int cmd, unsigned long ioctl_param);
long essw_fpga_device_ioctl (struct file *mfile, unsigned int cmd, unsigned long ioctl_param);

// Additional Function
unsigned char get_init_fnd (void);
void text_lcd_rotate (void);
void fnd_rotate (void);
void device_output_wrapper (void);
void device_clear (void);

// Function for timer
unsigned long timer_handler (unsigned long param);

unsigned char fpga_dot_number[10][10] = {
	{0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e}, // 0
	{0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
	{0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
	{0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
	{0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
	{0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03}  // 9
};

unsigned char fpga_dot_blank[10] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
