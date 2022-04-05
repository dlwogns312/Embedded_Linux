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
        printf("Error occured at allocating shared memory!\n");
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

    printf("free for shared memory\n");
    //Deallocate the shared memory
    if(pid_input&&pid_output)
    {
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
   //output_data->fnd_data=get_cur_time();

    while(!check_terminate)
    {
        printf("main\n");
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
            counter_process();
            break;
            default: break;
        }
    }
}
