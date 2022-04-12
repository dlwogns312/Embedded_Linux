#include "main_process.h"

static int now_mode=0;

//value for counter
int counter_num=0;
static int counter_mode=0;

//value for clock
static int clock_mode=0;
static int add_for_clock=0;
static int which_switch;

//value for text editor
static int text_input,same_cnt;
static int text_mode;
char* text_board[9]={
    ".QZ","ABC","DEF",
    "GHI","JKL","MNO",
    "PRS","TUV","WXY"
};
unsigned char fpga_number[11][10] = {
	{0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e}, // 0
	{0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
	{0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
	{0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
	{0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
	{0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03}, // 9
    {0x1c,0x36,0x63,0x63,0x63,0x7f,0x7f,0x63,0x63,0x63}  // A
};

//value for draw board
static int cursor_mode=0;
static int board_cnt,display_mode;
unsigned char dot_info[10];
static int blink,now_i,now_j;
#define max_dot (1<<7)-1

//Array for mode name
char* mode_print[]={"CLOCK","COUNTER","TEXT_EDITOR","DRAW_BOARD"};

void update_mode(SHM_OUTPUT* output_data,int readkey_input)
{
    
    if(readkey_input==READKEY_VOLUME_UP)
        now_mode=(now_mode+1)%4;
    else if(readkey_input==READKEY_VOLUME_DOWN)
        now_mode=(now_mode+3)%4;
    
    //Initiallize all the values used in each mode
    memset(output_data->display_dot,0,10);
    memset(output_data->text_data,0,32);
    switch(now_mode)
    {
        case 0:clock_mode=0;add_for_clock=0;which_switch=0;output_data->led=128;output_data->fnd_data=board_time();break;
        case 1:counter_mode=0;counter_num=0;output_data->fnd_data=0;output_data->led=64;break;
        case 2:output_data->led=0;same_cnt=0;text_input=0;text_mode=0; output_data->fnd_data=0;memcpy(output_data->display_dot,fpga_number[10],10);break;
        case 3:blink=0;now_i=0;now_j=6;output_data->led=0;display_mode=0;output_data->fnd_data=0;memset(dot_info,0,10);break;
    }
    output_data->mode=now_mode;
    printf("Changed to Mode %s!\n",mode_print[now_mode]);
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
    memset(output_data->text_data,0,32);
   //output_data->fnd_data=get_cur_time();

    while(!check_terminate)
    {
        semop(sem_id,&p[0],1);
        semop(sem_id,&p[1],1);
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

        //Operation separated by current mode
        switch(now_mode)
        {
            case 0:
            clock_process(output_data,input_data->switchkey);
            break;
            case 1:
            counter_process(output_data,input_data->switchkey);
            case 2:
            text_editor_process(output_data,input_data->switchkey);
            break;
            case 3:
            draw_board_process(output_data,input_data->switchkey);
            break;
            default: break;
        }
        semop(sem_id,&v[1],1);
        semop(sem_id,&v[0],1);
        usleep(100000);
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
            which_switch=0;
            printf("Editing Clock!\n");
        }
        else
        {
            printf("Done Editing Clock!\n");
            if(add_for_clock%100>=60)
            {
                add_for_clock+=100;
                add_for_clock-=add_for_clock%100;
            }
        }
    }
    else if(switchkey[1]==1)
    {
        switchkey[1]=0;
        add_for_clock=0;
    }
    else if(switchkey[2]==1&&clock_mode)
    {
        switchkey[2]=0;
        add_for_clock+=100;
    }
    else if(switchkey[3]==1&&clock_mode)
    {
        switchkey[3]=0;
        add_for_clock+=1;
        if(add_for_clock%100==60)
            add_for_clock+=40;
    }
    
    if(clock_mode)
    {
        which_switch++;
        if(which_switch>=10)
        {
            which_switch=0;
        }
        if(which_switch<5)
            output_data->led=16;
        else
            output_data->led=32;
    }
    else
    {
        output_data->led=128;
    }

    //Adjust hour max into 24 and min max into 60
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

//Text editor function
void text_editor_process(SHM_OUTPUT* output_data, unsigned char* switchkey)
{
    int i,j,input_check;
    int prev_input;

    input_check=0;

    if(switchkey[1]==1&&switchkey[2]==1)
    {
        //initialize switchkey and reset the text_editor mode
        same_cnt=0;
        switchkey[1]=0;
        switchkey[2]=0;

        memset(output_data->text_data,0,32);
        text_input=0;

        output_data->fnd_data=(output_data->fnd_data+1)%10000;
        return;
    }
    else if(switchkey[4]==1&&switchkey[5]==1)
    {
        text_mode=1-text_mode;
        switchkey[4]=0;switchkey[5]=0;

        if(text_mode==0)
        {
            printf("Change into Alphabet mode!\n");
            memcpy(output_data->display_dot,fpga_number[10],10);
        }
        else
        {
            printf("Change into Number mode!\n");
            memcpy(output_data->display_dot,fpga_number[1],10);
        }
        text_input=0;
        output_data->fnd_data=(output_data->fnd_data+1)%10000;

        return;
    }

    else if(switchkey[7]==1&&switchkey[8]==1)
    {
        switchkey[7]=0;switchkey[8]=0;
        same_cnt=0;
        text_input=0;

        for(j=1;j<32;j++)
            output_data->text_data[j-1]=output_data->text_data[j];
        output_data->text_data[31]=' ';
        
        output_data->fnd_data=(output_data->fnd_data+1)%10000;

         return;
    }

    for(i=0;i<9;i++)
    {
        if(switchkey[i]==1)
        {
            prev_input=text_input;
            text_input=i+1;switchkey[i]=0;
            input_check=1;
            output_data->fnd_data=(output_data->fnd_data+1)%10000;
            break;
        }
    }

    if(input_check==1)
    {
        if(text_mode==0)
        {
            if(text_input==prev_input)
            {
                same_cnt=(same_cnt+1)%3;
                output_data->text_data[31]=text_board[i][same_cnt];
            }
            else if(prev_input!=text_input)
            {
                same_cnt=0;
                for(j=1;j<32;j++)
                    output_data->text_data[j-1]=output_data->text_data[j];
                output_data->text_data[31]=text_board[i][same_cnt];
            }
        }
        else if(text_mode==1)
        {
            same_cnt=0;
            for(j=1;j<32;j++)
                output_data->text_data[j-1]=output_data->text_data[j];
            output_data->text_data[31]=(i+1)+'0';
            
        }
    }
}

//draw board function
void draw_board_process(SHM_OUTPUT* output_data,unsigned char* switchkey)
{
    int select=0;
    int ret;

    blink=(blink+1)%10;
    if(switchkey[0]==1)
    {
        switchkey[0]=0;
        memset(dot_info,0,10);
        board_cnt=0;
        now_i=0;now_j=6; display_mode=0;blink=0;cursor_mode=0;
    }
    else if(switchkey[1]==1)
    {
        switchkey[1]=0;
        board_cnt++;
        now_i--;
        if(now_i<0)
            now_i=9;
    }
    else if(switchkey[2]==1)
    {
        switchkey[2]=0;
        cursor_mode=1-cursor_mode;
        blink=0;
        board_cnt++;
    }
    else if(switchkey[3]==1)
    {
        switchkey[3]=0;
        board_cnt++;
        now_j++;
        if(now_j>6)
            now_j=0;
    }
    else if(switchkey[4]==1)
    {
        switchkey[4]=0;
        select=1;
        board_cnt++;
    }
    else if(switchkey[5]==1)
    {
        switchkey[5]=0;
        now_j--;
        if(now_j<0)
            now_j=6;
        board_cnt++;
    }
    else if(switchkey[6]==1)
    {
        switchkey[6]=0;
        memset(dot_info,0,10);
        board_cnt++;
    }
    else if(switchkey[7]==1)
    {
        switchkey[7]=0;
        now_i++;
        if(now_i>9)
            now_i=0;
        board_cnt++;
    }
    else if(switchkey[8]==1)
    {
        switchkey[8]=0;
        board_cnt++;
        display_mode=1-display_mode;
    }

    if(select)
        dot_info[now_i]|=(1<<now_j);

    memcpy(output_data->display_dot,dot_info,10);

    output_data->display_dot[now_i]|=(1<<now_j);

    if(blink<5||cursor_mode)
        output_data->display_dot[now_i]-=(1<<now_j);

    int i;
    if(display_mode)
    {
        for(i=0;i<10;i++)
            output_data->display_dot[i]^=max_dot;
    }
    output_data->fnd_data=board_cnt;  
}