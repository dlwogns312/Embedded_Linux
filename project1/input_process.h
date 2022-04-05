#ifndef __INPUT_PROCESS__
#define __INPUT_PROCESS__

#include <linux/input.h>
#include <stdio.h>
#include <string.h>

#include "main.h"

void input_process(int shm_input);
void operating_readkey(int readkey,SHM_INPUT* shm_temp;
void operating_switchkey(int switchkey,SHM_INPUT* shm_temp);

#endif