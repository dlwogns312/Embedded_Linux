#include "input_process.h"

void input_process(int shm_input)
{
    //for debug
    printf("Input_process Successfully loaded!\n");

    int readkey,switchkey;
    int check_terminate=0;

    //Mapping the address of shared memory
    SHM_INPUT *shm_temp = (SHM_INPUT*)shmat(shm_input,(char*)NULL,0);
    
    //Open readkey and switch key file using location
    char* read_pos="/dev/input/event0";
    char* switch_pos="/dev/fpga_push_switch";
    readkey=open(read_pos,O_RDONLY|O_NONBLOCK);
    switchkey=open(switch_pos,O_RDWR);

    if(readkey<0)
    {
        printf("Readkey Error!\n");
        exit(1);
    }
    if(switchkey<0)
    {
        printf("SwitchKey Error!\n");
        exit(1);
    }

    while(!check_terminate)
    {
        if(shm_temp->check_terminate)
            check_terminate=1;

        //input for readkey and switchkey
        operating_readkey(readkey,shm_temp);
        operating_switchkey(switchkey,shm_temp);
        
        usleep(100000);
    }

    shmdt((char *)shm_temp);
    close(readkey);close(switchkey);

    printf("Input_process Out!\n");
    return;
}

void operating_readkey(int readkey,SHM_INPUT* shm_temp)
{
    struct input_event ev[64];
    int num=sizeof(struct input_event);
    int fd=read(readkey,ev,num*BUFF_SIZE);

    if(fd>0&&ev[0].value==KEY_PRESS)
    {
        switch(ev[0].code)
        {
            case READKEY_BACK:
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

    return;
}

void operating_switchkey(int switchkey, SHM_INPUT* shm_temp)
{
    unsigned char temp[9];
    unsigned char prev_temp[9],result[9];

    int num=sizeof(unsigned char)*9;
    int i,j;

    memset(result,0,num);

    for(i=0;i<5000;i++)
    {
        memcpy(prev_temp,temp,num);
        read(switchkey,temp,num);

        for(j=0;j<9;j++)
        {
            if(temp[j]-prev_temp[j]==KEY_PRESS)
                result[j]=KEY_PRESS;
        }
    }

    memcpy(shm_temp->switchkey,result,num);

    return;
}