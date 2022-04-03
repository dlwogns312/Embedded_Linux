#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#include "main.h"

#define MAX_DIGIT 4
//FPGA addresss
#define FPGA_BASE_ADDRESS 0x08000000 
//LED address
#define LED_ADDR 0x16

//Define locations of output device
#define FND_DEVICE "/dev/fpga_fnd"
#define FPGA_DOT_DEVICE "/dev/fpga_dot"
#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"

//function for output device
void device_lcd (unsigned char data[32]);
void device_fnd (int data);
void device_led(unsigned char data);
void device_dot (unsigned char data[10]);