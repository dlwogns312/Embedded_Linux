#include "main.h"

int main(void)
{
    //Value for SHM and PID
    int shm_input,shm_output;
    pid_t pid_input, pid_output,pid_main;

    //Allocate for shared memory
    shm_input=shmget(KEY_INPUT, sizeof(SHM_INPUT),0600|IPC_CREAT);
    shm_output=shmget(KEY_OUTPUT,sizeof(SHM_OUTPUT),0600|IPC_CREAT);

    if(shm_input==-1||shm_output==-1)
    {
        printf("Error occured at allocating shared memory! input: %d, output:%d\n",shm_input,shm_output);
        exit(-1);
    }

    //get semaphore
    sem_id=semget(SEMA_KEY,SEMA_MAX,IPC_CREAT);
    if(sem_id==-1)
    {
        printf("Error occured at getting semaphore id!\n");
        exit(-1);
    }

    int i;
    union semun op;
    op.val=1;

    //initilize semaphore
    for(i=0;i<SEMA_MAX;i++)
    {
        if(semctl(sem_id,i,SETVAL,op)==-1)
        {
            printf("Error occured at setting semaphore!\n");
            exit(-1);
        }
        p[i].sem_num=v[i].sem_num=i;
        p[i].sem_flg=v[i].sem_flg=SEM_UNDO;

        p[i].sem_op=-1;v[i].sem_op=1;
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
        else
        {
            pid_main=fork();
            if(pid_main==-1)
            {
                printf("Error occured at making pid!\n");
                exit(-1);
            }
            if(!pid_main)
                main_process(shm_input, shm_output);
        }
    }

    //wait for child process
    wait(NULL);
    wait(NULL);
    wait(NULL);
   
    //Deallocate the shared memory
    if(pid_input&&pid_output&&pid_main)
    {
        printf("free for shared memory\n");
        shmctl(shm_input,IPC_RMID,NULL);
        shmctl(shm_output,IPC_RMID,NULL);

        printf("deallocate semaphore!\n");
        semctl(sem_id,0,IPC_RMID,0);
    }
    return 0;
}
