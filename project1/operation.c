#include "operation.h"

extern int counter_num;
void counter_process (SHM_OUTPUT* shm_output, unsigned char* switchkey,int* now_mode)
{
    if(switchkey[0]==1)
    {
        switchkey[0]=0;
        *now_mode=(*now_mode+1)%4;
        if(*now_mode==0)
            shm_output->led=128;
        else
            shm_output->led/=2;

        convert_base(shm_output,now_mode);
    }
    else if(switchkey[1]==1)
    {
        switchkey[1]=0;
        digit_update(shm_output,1,now_mode);
    }
    else if(switchkey[2]==1)
    {
        switchkey[2]=0;
        digit_upadate(shm_output,1,now_mode);
    }
    else if(switchkey[3]==1)
    {
        switchkey[3]=0;
        digit_update(shm_output,1,now_mode);
    }

    convert_base(shm_output,now_mode);
}

void digit_update(SHM_OUTPUT *shm_output,int digit, int* now_mode)
{
    int temp;

    switch(*now_mode)
    {
        case 0:
            temp=10;break;
        case 1:
            temp=8;break;
        case 2:
            temp=4;break;
        case 3:
            temp=2; break;
    }

    switch(digit)
    {
        case 3:
            counter_num++;break;
        case 2:
            counter_num+=temp;break;
        case 1:
            counter_num+=temp*temp;break;

    }

    return;
}
void convert_base(SHM_OUTPUT* shm_output, int* now_mode)
{
    int temp;

     switch(*now_mode)
    {
        case 0:
            temp=10;break;
        case 1:
            temp=8;break;
        case 2:
            temp=4;break;
        case 3:
            temp=2; break;
    }

    int third_digit=(counter_num%temp);
    int second_digit=(counter_num/temp)%temp;
    int first_digit=(counter_num/temp/temp)%temp;

    temp=first_digit*100+second_digit*10+third_digit;
    shm_output->fnd_data=temp;

    return;
}