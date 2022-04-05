#include "output_process.h"

void output_process(int shm_output)
{
    printf("Output_process successfully Loaded!\n");

    int check_terminate=0;
    int now_mode;
    SHM_OUTPUT* data_out=(SHM_OUTPUT*)shmat(shm_output,(char*)NULL,0);

    while(!check_terminate)
    {
        if(data_out->check_terminate)
        {
            data_out->fnd_data=0;
            data_out->led=0;
            check_terminate=1;
        }

        now_mode=data_out->mode;

        switch(now_mode)
        {
            case 0:break;
            case 1:
            device_fnd(data_out->fnd_data);
            device_led(data_out->led);
            break;
        }

        usleep(100000);
    }
    shmdt((char*)data_out);

    printf("Output_process out!\n");

    return;
}