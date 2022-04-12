#ifndef __MAIN_PROCESS__
#define __MAIN_PROCESS__

#include "main.h"

void main_process(int shm_input, int shm_output);

//get board time
int board_time();

//Mode 1 - Clock
void clock_process(SHM_OUTPUT* output_data,unsigned char* switchkey);


//Mode 2 - Counter
void counter_process(SHM_OUTPUT* output_data,unsigned char* switchkey);
void digit_update(SHM_OUTPUT* output_datat,int digit);
void convert_base(SHM_OUTPUT* output_data);

//Mode 3 - Text editor
void text_editor_process(SHM_OUTPUT* output_data,unsigned char* switchkey);

//Mode 4 - Draw Board
void draw_board_process(SHM_OUTPUT* output_data,unsigned char* switchkey);




#endif