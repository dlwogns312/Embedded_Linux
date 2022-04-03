#include "device.h"

void device_lcd (unsigned char * data)
{

}

void device_fnd(int fnd_data)
{
    unsigned char save[4];
    int i;
    int dev;

    dev=open(FND_DEVICE,O_RDWR);
    if(dev<0)
    {
        printf("Device open error: %s\n",FND_DEVICE);
        exit(1);
    }

    for(i=3;i>=0;i--)
    {
        save[i]=fnd_data%10;
        fnd_data/=10;
    }

    unsigned char for_write=write(dev,&save,4);
    if(for_write<0)
    {
        printf("Write Error!\n");
        return;
    }

    close(dev);
}

void device_led(unsigned char led)
{
    int fd,i;
    unsigned long *fpga_addr = 0;
    unsigned char *led_addr =0;

    fd=open("dev/mem",O_RDWR|O_SYNC);
    if(fd<0)
    {
        perror("/dev/mem open error");
        exit(1);
    }

    fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FPGA_BASE_ADDRESS);
	if (fpga_addr == MAP_FAILED)
	{
		printf("mmap error!\n");
		close(fd);
		exit(1);
	}
	
	led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);
	
	*led_addr=led; //write led
	
	munmap(led_addr, 4096);
	close(fd);

    usleep(1000);
	return 0;
}