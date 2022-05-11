/*  
    Name : Device Driver,
    Author : Seungjin Lee,
    Date : 2020-05-18,
    Description : FPGA Device Driver using Timer 
*/ 

#include "dev_driver.h"

//  Global variable
//  Usage counter
//  I/O Port Addr pointer
static int essw_fpga_port_usage = 0;

static unsigned char *fpga_text_lcd_addr;
static unsigned char *fpga_dot_addr;
static unsigned char *fpga_led_addr;
static unsigned char *fpga_fnd_addr;

static unsigned char timer_count;

// Variables for FND Rotation
static unsigned char fnd_idx;
static unsigned char fnd_count = 0;

// Variables for TEXT_LCD Rotation
static unsigned char upper_dir = RIGHT;
static unsigned char upper_idx = 0;
static unsigned char lower_dir = RIGHT;
static unsigned char lower_idx = 0;

// Current Output Values
unsigned char init_fnd_val;
unsigned char fnd_value[4];
unsigned char text_lcd_value[32];

// define file_operations structure 
struct file_operations essw_fpga_device_fops =
{
	owner:		        THIS_MODULE,
	open:		        essw_fpga_device_open,
	unlocked_ioctl:		essw_fpga_device_ioctl,	
	release:	        essw_fpga_device_release,
};

struct timer_list timer;
struct data_packet my_data;

// [Q] Can Timer be static? 
// 'cause it's acting like call back in timer interrupt, I guess not but ...?
unsigned long timer_handler (unsigned long param) 
{   
    struct data_packet *tmp_data = (struct data_packet*) param;  
    // printk("Timer Count is %d\n", timer_count);

    if(timer_count > 100){
        printk ("Error! Probably Overflow from other variables\n");
        return -EFAULT;
    }

    // If Timer done,
    if (timer_count == 0) {
        printk ("Timer Done\n");
        memset (fnd_value, 0, 4);
        memset (text_lcd_value, ' ', 32);
        device_clear ();
    } 
    else { // Else, update the values 
        fnd_rotate ();
        text_lcd_rotate ();
        device_output_wrapper ();
        timer_count--;
        
        // and set timer value for next tick
        timer.expires = get_jiffies_64() + (tmp_data->interval * HZ / 10);
        timer.function = (void *) timer_handler;
        timer.data = (unsigned long) tmp_data;
        add_timer (&timer);
    }

    return 0;
}


int essw_fpga_device_open(struct inode *minode, struct file *mfile) 
{	
	if (essw_fpga_port_usage != 0) return -EBUSY;

    printk (KERN_ALERT "Device Open!\n");
	essw_fpga_port_usage = 1;
	return 0;
}

int essw_fpga_device_release(struct inode *minode, struct file *mfile) 
{
    printk (KERN_ALERT "Device Release!\n");
    essw_fpga_port_usage = 0;
	return 0;
}

// int essw_fpga_device_ioctl (struct inode *minode, struct file *mfile, unsigned int cmd, unsigned long ioctl_param)
long essw_fpga_device_ioctl (struct file *mfile, unsigned int cmd, unsigned long ioctl_param) // (Kernel Version > 2.6.5)
{
    // Copy the parameter of ioctl through copy_from_user!
	if (copy_from_user(&my_data, (struct data_packet*)ioctl_param, sizeof(struct data_packet)))
		return -EFAULT;

    // printk(KERN_ALERT "interval : %d count : %d FND_INIT : %d%d%d%d\n", \
    //     my_data.interval, my_data.cnt, my_data.init[0], my_data.init[1], my_data.init[2], my_data.init[3]);
       
    if (cmd == ESSW_DEVICE_SET)
    {
        // Init all the global variables
        fnd_count = fnd_idx = init_fnd_val = 0;;
        upper_idx = 0; upper_dir = RIGHT;
        lower_idx = 16; lower_dir = RIGHT;

        // Init Output Values
        printk (KERN_ALERT "IOCTL with ESSW_DEVICE_SET CALLED!\n");
        memcpy (fnd_value, my_data.init, 4);
        timer_count = my_data.cnt - 1;
        init_fnd_val = get_init_fnd ();

        // Set the Text LCD
        memset (text_lcd_value, 0, 32);        
		strncat (text_lcd_value, "20141455", 8);
		memset (text_lcd_value+8,' ', 8);
		strncat (text_lcd_value, "Seungjin Lee", 12);
		memset (text_lcd_value+28,' ', 4);

        // Output the initial values
        device_output_wrapper ();
    }
    else if (cmd == ESSW_DEVICE_OPERATE)
    {
        // Start Timer!
        printk (KERN_ALERT "IOCTL with ESSW_DEVICE_OPERATE CALLED!\n");
        timer.expires = get_jiffies_64 () + (my_data.interval * HZ / 10);
        timer.function = (void *) timer_handler;
        timer.data = (unsigned long) &my_data;

        printk (KERN_ALERT "TIMER START!\n");
        add_timer (&timer);
    }
    else { // Error Invalid IOCTL Command
        printk (KERN_WARNING "dev_driver::IOCTL ERROR! Invalid Command\n");
        return -EFAULT;
    }

	return 0;
}

// Init done!
int __init essw_fpga_device_init(void)
{
	int result;

	result = register_chrdev (ESSW_DEVICE_MAJOR, ESSW_DEVICE_NAME, &essw_fpga_device_fops);

	if(result < 0) {
		printk (KERN_WARNING "Can't get any major\n");
		return result;
	}

    // Init timer and ioremap all the devices
    init_timer (&timer);
    fpga_text_lcd_addr  = ioremap (FPGA_TEXT_LCD_ADDRESS, 0x32);
    fpga_fnd_addr       = ioremap (FPGA_FND_ADDRESS, 0x4);
    fpga_led_addr       = ioremap (FPGA_LED_ADDRESS, 0x1);
    fpga_dot_addr       = ioremap (FPGA_DOT_ADDRESS, 0x10);

    if (fpga_text_lcd_addr == NULL || fpga_fnd_addr == NULL || \
        fpga_led_addr == NULL || fpga_dot_addr == NULL) 
    {
        printk (KERN_WARNING "dev_init::INIT ioremap for io device error!\n");
        return -EFAULT;
    }

	printk ("init module, %s major number : %d\n", ESSW_DEVICE_NAME, ESSW_DEVICE_MAJOR);
	return 0;
}

void __exit essw_fpga_device_exit(void) 
{
    /* IO Unmap for 4 I/O Devices */
    iounmap (fpga_text_lcd_addr);
    iounmap (fpga_dot_addr);
    iounmap (fpga_led_addr);
    iounmap (fpga_fnd_addr);

    del_timer_sync(&timer);
	unregister_chrdev (ESSW_DEVICE_MAJOR, ESSW_DEVICE_NAME);
}

module_init (essw_fpga_device_init);
module_exit (essw_fpga_device_exit);
MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Seungjin Lee");

/* 
    Additional function for driver operation
*/

unsigned char get_init_fnd (void) {

    int i;
    
    // Find Initial Start Values of FND
    for (i=0; i<4; i++)
        if (fnd_value[i] != 0) {
            fnd_idx = i;
            return fnd_value[i];
        }

    printk (KERN_WARNING "Error with FND Value!\n");
    return -1;
}

void fnd_rotate (void) {

    int i;

    // Get the current digit location
    for (i=0; i<4; i++)
        if (fnd_value[i] != 0) {
            fnd_idx = i;
            break;
        }

    // Increment fnd_count in a circular way
    if (fnd_count == 7) {
        fnd_count = 0;

        fnd_value[fnd_idx] = 0;
        fnd_idx = (fnd_idx + 1) % 4;
        fnd_value[fnd_idx] = init_fnd_val;
    }
    else {
        fnd_count++;      
        fnd_value[fnd_idx] = fnd_value[fnd_idx] % 8 + 1;
    }
}

// Need to get polished!
void text_lcd_rotate (void) {

    int i;

    // Upper Part : Student ID
    switch (upper_dir) {
        case RIGHT: // If it doens't hit the right wall
            if (upper_idx < 16 - ID_LEN) {
                for (i=upper_idx+ID_LEN; i > upper_idx; i--) {
                    text_lcd_value[i] = text_lcd_value[i-1];
                }
                
                text_lcd_value[upper_idx++] = ' ';
            } 
            else { // If it hits the right wall               
                for (i=upper_idx-1; i<upper_idx+ID_LEN; i++){
                    text_lcd_value[i] = text_lcd_value[i+1];
                }

                text_lcd_value[upper_idx+ID_LEN-1] = ' ';
                upper_idx--;
                upper_dir = LEFT;
            } 
            break;

        case LEFT: // If it doens't hit the left wall
            if (upper_idx > 0) {
                for (i=upper_idx-1; i<upper_idx+ID_LEN; i++){
                    text_lcd_value[i] = text_lcd_value[i+1];
                }

                text_lcd_value[upper_idx+ID_LEN] = ' ';
                upper_idx--;
            } 
            else { // If it hits the left wall 
                for (i=upper_idx+ID_LEN; i > upper_idx; i--) {
                    text_lcd_value[i] = text_lcd_value[i-1];
                }
                
                text_lcd_value[upper_idx++] = ' ';
                upper_dir = RIGHT;
            }             
            break;
    }

    // Lower Part : NAME
    switch (lower_dir) {
        case RIGHT: // If it doens't hit the right wall
            if (lower_idx < 32-NAME_LEN) {
                for (i=lower_idx+NAME_LEN; i > lower_idx; i--)
                    text_lcd_value[i] = text_lcd_value[i-1];
                
                text_lcd_value[lower_idx++] = ' ';
            }
            else { // If it hits the right wall
                for (i=lower_idx-1; i<lower_idx+NAME_LEN; i++)
                    text_lcd_value[i] = text_lcd_value[i+1];
                

                text_lcd_value[lower_idx+NAME_LEN-1] = ' ';
                lower_idx--;
                lower_dir = LEFT;
            }
            break;

        case LEFT: // If it doens't hit the left wall
            if (lower_idx > 16) {
                for (i=lower_idx-1; i<lower_idx+NAME_LEN; i++)
                    text_lcd_value[i] = text_lcd_value[i+1];
                

                text_lcd_value[lower_idx+NAME_LEN] = ' ';
                lower_idx--;
            } 
            else { // If it hits the left wall
                for (i=lower_idx+NAME_LEN; i > lower_idx; i--) 
                    text_lcd_value[i] = text_lcd_value[i-1];
                
                
                text_lcd_value[lower_idx++] = ' ';
                lower_dir = RIGHT;
            }             
            break;
    }
}

void device_output_wrapper (void) {
    
    int i;
    unsigned short int short_val;

    // Text LCD out
    for (i=0; i<33; i+=2) {
        short_val = (text_lcd_value[i] & 0xFF) << 8 | (text_lcd_value[i+1] & 0xFF);
        outw (short_val, (unsigned int)fpga_text_lcd_addr+i);
    }

    // FND LED out
    short_val = fnd_value[0] << 12 | fnd_value[1] << 8 | fnd_value[2] << 4 | fnd_value[3];
    outw (short_val, (unsigned int)fpga_fnd_addr);

    // LED out
    short_val = 0x80 >> (fnd_value[fnd_idx]-1);
    outw (short_val, (unsigned int)fpga_led_addr);

    // Dot Matrix out
    for (i=0; i<10; i++) {
        short_val = fpga_dot_number[fnd_value[fnd_idx]][i] & 0x7F;
		outw (short_val, (unsigned int)fpga_dot_addr + i*2);
    }
}

void device_clear (void) {
    
    int i;
    unsigned short int short_val;

    // Text LCD out
    for (i=0; i<33; i+=2) {
        short_val = (text_lcd_value[i] & 0xFF) << 8 | (text_lcd_value[i+1] & 0xFF);
        outw (short_val, (unsigned int)fpga_text_lcd_addr+i);
    }

    // FND LED out
    short_val = fnd_value[0] << 12 | fnd_value[1] << 8 | fnd_value[2] << 4 | fnd_value[3];
    outw (short_val, (unsigned int)fpga_fnd_addr);

    // LED Off
    short_val = 0;
    outw (short_val, (unsigned int)fpga_led_addr);

    // Dot Matrix off
    for (i=0; i<10; i++) {
        short_val = fpga_dot_blank[i] & 0x7F;
		outw (short_val, (unsigned int)fpga_dot_addr + i*2);
    }
}