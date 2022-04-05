#include "main.h"

static int now_mode=1;
static int counter_mode=0;
static int clock_mode=0;
int counter_num=0;

int main(void)
{
    //Value for SHM and PID
    int shm_input,shm_output;
    pid_t pid_input, pid_output;

    //Allocate for shared memory
    shm_input=shmget(KEY_INPUT, sizeof(SHM_INPUT),0600|IPC_CREAT);
    shm_output=shmget(KEY_OUTPUT,sizeof(SHM_OUTPUT),0600|IPC_CREAT);

    if(shm_input==-1||shm_output==-1)
    {
        printf("Error occured at allocating shared memory! input: %d, output:%d\n",shm_input,shm_output);
        exit(-1);
    }
    
    pid_input=fork();
    if(pid_input==-1)
    {
        printf("Error occured at making pid!\n");
        exit(-1);
    }
    
    if(!pid_input)
    {
        input_process(shm_input);
    }
    else{
        pid_output=fork();
        if(pid_output==-1)
        {
            printf("Error occured at making pid!\n");
            exit(-1);
        }
        if(!pid_output)
            output_process(shm_output);
    }

    //Start main_process if pid_input and pid_output are successfully generated
    if(pid_input&&pid_output)
    {
        main_process(shm_input,shm_output);
    }


    //wait for child process
    wait(NULL);
    wait(NULL);

   
    //Deallocate the shared memory
    if(pid_input&&pid_output)
    {
        printf("free for shared memory\n");
        shmctl(shm_input,IPC_RMID,NULL);
        shmctl(shm_output,IPC_RMID,NULL);
    }
    return;
}

void main_process(int shm_input, int shm_output)
{

    printf("Main_process started!\n");

    int check_terminate=0;
    int readkey_input,readkey_prev;
    int i;
    SHM_INPUT *input_data=(SHM_INPUT*)shmat(shm_input,(char*)NULL,0);
    SHM_OUTPUT* output_data=(SHM_OUTPUT*)shmat(shm_output,(char*)NULL,0);

    //initialize the input data
    input_data->readkey=-1;
    input_data->check_terminate=0;
    for(i=0;i<9;i++)
        input_data->switchkey[i]=0;
    
    //initialize the output data
    output_data->check_terminate=0;
    output_data->led=128;
    output_data->real_init=1;
   //output_data->fnd_data=get_cur_time();

    while(!check_terminate)
    {
        readkey_prev=readkey_input;
        readkey_input=input_data->readkey;

        if(readkey_input!=readkey_prev)
        {
            switch(readkey_input)
            {
                case READKEY_VOLUME_UP: break;
                case READKEY_VOLUME_DOWN: break;

                case READKEY_BACK:
                    check_terminate=1;
                    input_data->check_terminate=1;
                    output_data->check_terminate=1;
                    break;
                default: break;
                
            }
        }

        usleep(1000);

        //Operation separated by current mode
        switch(now_mode)
        {
            case 0:
            break;
            case 1:
            counter_process(output_data,input_data->switchkey,&counter_mode);
            break;
            default: break;
        }

    }

    shmdt((char*)input_data);
    shmdt((char*)output_data);
    printf("main ended!\n");
}

//counter_function
void counter_process (SHM_OUTPUT* output_data, unsigned char* switchkey,int* now_mode)
{
    int i;
    printf("%s\n",switchkey);

    if(switchkey[0]==1)
    {
        switchkey[0]=0;
        *now_mode=(*now_mode+1)%4;
        if(*now_mode==0)
            output_data->led=128;
        else
            output_data->led/=2;

        convert_base(output_data,now_mode);
    }
    else if(switchkey[1]==1)
    {
        switchkey[1]=0;
        digit_update(output_data,1,now_mode);
    }
    else if(switchkey[2]==1)
    {
        switchkey[2]=0;
        digit_update(output_data,1,now_mode);
    }
    else if(switchkey[3]==1)
    {
        switchkey[3]=0;
        digit_update(output_data,1,now_mode);
    }

    convert_base(output_data,now_mode);
}

void digit_update(SHM_OUTPUT *output_data,int digit, int* now_mode)
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

void convert_base(SHM_OUTPUT* output_data, int* now_mode)
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
    output_data->fnd_data=temp;

    return;
}
