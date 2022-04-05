#include "output_process.h"

void output_process(int shm_output)
{
    printf("Output_process successfully Loaded!\n");

    int check_terminate=0;
    int now_mode;
    
    SHM_OUTPUT* data_out=(SHM_OUTPUT*)shmat(shm_output,(char*)NULL,0);

    while(!check_terminate)
    {
        //sleep(1);
        
        //printf("output\n");
        if(data_out->check_terminate)
        {
            data_out->fnd_data=0;
            data_out->led=0;
            check_terminate=1;
        }

        now_mode=data_out->mode;

        switch(now_mode)
        {
            case 0:
            device_fnd(data_out->fnd_data);
            device_led(data_out->led);
            break;
            case 1:
            device_fnd(data_out->fnd_data);
            device_led(data_out->led);
            case 2:
            device_fnd(data_out->fnd_data);
            device_led(data_out->led);
            case 3:
            device_fnd(data_out->fnd_data);
            break;

        }

        usleep(100000);
    }
    shmdt((char*)data_out);

    printf("Output_process out!\n");

    return;
}

void device_lcd (unsigned char * data)
{

}

void device_fnd(int fnd_data)
{
    unsigned char save[4];
    int i;
    int dev;
    char* fnd_pos="/dev/fpga_fnd";
    dev=open(fnd_pos,O_RDWR);

    if(dev<0)
    {
        printf("Device open error: %s\n",fnd_pos);
        exit(1);
    }
    //printf("%d\n",fnd_data);

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

    //printf("%d\n",led);
    fd=open("/dev/mem",O_RDWR|O_SYNC);
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

	//write led
	*led_addr=led; 
	
	munmap(led_addr, 4096);
	close(fd);

    usleep(1000);
	return ;
}