#include "input_process.h"

void input_process(int shm_input)
{
    printf("Input_process Successfully loaded!\n");
    int readkey,switchkey;
    int check_terminate=0;
    int i,j;
    //Mapping the address of shared memory
    SHM_INPUT *shm_temp = (SHM_INPUT*)shmat(shm_input,(char*)NULL,0);
    
    //Open readkey and switch key file using location
    readkey=open(DEVICE_READKEY,O_RDONLY|O_NONBLOCK);
    switchkey=open(DEVICE_SWITCH,O_RDWR);

    while(!check_terminate)
    {
        printf("input\n");
        if(shm_temp->check_terminate)
            check_terminate=1;
        
        struct input_event ev[64];
        int num=sizeof(struct input_event);
        int fd=read(readkey,ev,num*BUFF_SIZE);

        if(fd>0&&ev[0].value==KEY_DOWN)
        {
            switch(ev[0].code)
            {
                case READKEY_BACK:
                    check_terminate=1;
                    shm_temp->check_terminate=1;
                    break;
                case READKEY_VOLUME_UP:
                    shm_temp->readkey=ev[0].code;
                    break;
                case READKEY_VOLUME_DOWN:
                    shm_temp->readkey=ev[0].code;
                    break;
                default:
                    break;
            }
        }
        else
            shm_temp->readkey=-1;

        unsigned char temp[9];
        unsigned char prev_temp[9],result[9];

        num=sizeof(unsigned char)*9;
        
        memset(result,0,num);
        for(i=0;i<5000;i++)
        {
            memcpy(prev_temp,temp,num);
            read(switchkey,temp,num);

            for(j=0;j<9;j++)
            {
                if(temp[j]-prev_temp[j]==KEY_DOWN)
                    result[j]=KEY_DOWN;
            }
        }

        memcpy(shm_temp->switchkey,result,num);

    }

    shmdt((char *)shm_temp);
    close(readkey);close(switchkey);

    printf("Input_process Out!\n");
    return;
}