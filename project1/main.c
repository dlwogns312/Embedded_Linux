#include "main.h"

static int now_mode=0;
static int counter_mode=0;
static int clock_mode=0;
int counter_num=0;
static int add_for_clock=0,clock_temp=0;
static int prev_clock,which_switch;

//Array for mode name
char* mode_print[4]={"CLOCK","COUNTER","DRAW_BOARD","TEXT_EDITOR"};

void update_mode(SHM_OUTPUT* output_data,int readkey_input)
{
    
    if(readkey_input==READKEY_VOLUME_UP)
        now_mode=(now_mode+1)%4;
    else if(readkey_input==READKEY_VOLUME_DOWN)
        now_mode=(now_mode+3)%4;
    switch(now_mode)
    {
        case 0:prev_clock=board_time();clock_mode=0;clock_temp=0;add_for_clock=0;output_data->led=128;output_data->fnd_data=board_time();break;
        case 1:counter_mode=0;which_switch=0;counter_num=0;output_data->fnd_data=0;output_data->led=64;break;
        case 2:output_data->led=128;break;
        case 3:output_data->led=128;break;
    }
    output_data->mode=now_mode;
    printf("Changed to Mode %s!\n",mode_print[now_mode]);
}

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
    memset(input_data->switchkey,0,sizeof(input_data->switchkey));
    
    //initialize the output data
    output_data->check_terminate=0;
    output_data->fnd_data=board_time();
    output_data->led=128;
    output_data->mode=now_mode;
   //output_data->fnd_data=get_cur_time();

    while(!check_terminate)
    {

        readkey_prev=readkey_input;
        readkey_input=input_data->readkey;
       
        //Detect Readkey difference
        if(readkey_input!=readkey_prev)
        {
            switch(readkey_input)
            {
                case READKEY_VOLUME_UP: update_mode(output_data,readkey_input);break;
                case READKEY_VOLUME_DOWN: update_mode(output_data,readkey_input);break;

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
            clock_process(output_data,input_data->switchkey);
            break;
            case 1:
            counter_process(output_data,input_data->switchkey);
            break;
            default: break;
        }

    }

    shmdt((char*)input_data);
    shmdt((char*)output_data);
    printf("main ended!\n");
}

//clock_function
void clock_process (SHM_OUTPUT* output_data, unsigned char* switchkey)
{
    if(switchkey[0]==1)
    {
        switchkey[0]=0;
        clock_mode=1-clock_mode;
        if(clock_mode)
        {
            prev_clock=board_time();
            which_switch=0;
            printf("Editing Clock!\n");
        }
        else
        {
            printf("Done Editing Clock!\n");
            add_for_clock+=clock_temp;
            if(add_for_clock%100>=60)
            {
                add_for_clock+=100;
                add_for_clock-=add_for_clock%100;
            }
            clock_temp=0;
        }
    }
    else if(switchkey[1]==1)
    {
        switchkey[1]=0;
        clock_temp=0;add_for_clock=0;
    }
    else if(switchkey[2]==1&&clock_mode)
    {
        switchkey[2]=0;
        clock_temp+=100;
    }
    else if(switchkey[3]==1&&clock_mode)
    {
        switchkey[3]=0;
        clock_temp+=1;
        if(clock_temp%100==60)
            clock_temp+=40;
    }
    
    if(clock_mode)
    {
        if(board_time()-prev_clock>=1)
        {
            which_switch=1-which_switch;
            prev_clock=board_time();
        }
        if(which_switch)
            output_data->led=16;
        else
            output_data->led=32;
    }
    else
    {
        output_data->led=128;
    }
    int print_clock;
    print_clock=board_time()+add_for_clock;
    if(print_clock%100>=60)
    {
        print_clock+=100;
        print_clock-=print_clock%100;
    }
    print_clock=(print_clock)%2400;
    output_data->fnd_data=print_clock;
}


//counter_function
void counter_process (SHM_OUTPUT* output_data, unsigned char* switchkey)
{
    if(switchkey[0]==1)
    {
        switchkey[0]=0;
        counter_mode=(counter_mode+1)%4;
        switch(counter_mode)
        {
            case 0 :output_data->led=64;break;
            case 1: output_data->led=32;break;
            case 2: output_data->led=16;break;
            case 3: output_data->led=128;break;
        }
        convert_base(output_data);
    }
    else if(switchkey[1]==1)
    {
        switchkey[1]=0;
        digit_update(output_data,1);
    }
    else if(switchkey[2]==1)
    {
        switchkey[2]=0;
        digit_update(output_data,2);
    }
    else if(switchkey[3]==1)
    {
        switchkey[3]=0;
        digit_update(output_data,3);
    }

    convert_base(output_data);
}

void digit_update(SHM_OUTPUT *output_data,int digit)
{
    int temp;

    switch(counter_mode)
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

void convert_base(SHM_OUTPUT* output_data)
{
    int temp;
    switch(counter_mode)
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

int board_time()
{
    time_t t=time(NULL);
    struct tm tm=*localtime(&t);

    return 100*tm.tm_hour + tm.tm_min;
}
