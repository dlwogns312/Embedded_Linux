#include <time.h>
#include <string.h>

#include "main.h"

void counter_process(SHM_OUTPUT* shm_output,unsigned char* switchkey,int* now_mode);

void digit_update( SHM_OUTPUT* shm_output,int digit,int* now_mode);
void convert_base(SHM_OUTPUT* shm_output,int* now_mode);