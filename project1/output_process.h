#ifndef __OUTPUT_PROCESS__
#define __OUTPUT_PROCESS__

#include "main.h"
#include <sys/mman.h>

#define MAX_DIGIT 4
//FPGA addresss
#define FPGA_BASE_ADDRESS 0x08000000 
//LED address
#define LED_ADDR 0x16

//function for output device
void device_lcd (unsigned char data[32]);
void device_fnd (int data);
void device_led(unsigned char data);
void device_dot (unsigned char data[10]);

void output_process(int shm_output);

#endif