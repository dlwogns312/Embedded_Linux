#include "app.h"

int main(int argc, char* argv[]) {

    int fd, i, count = 0;
    
    struct data_packet my_data;
    
	if (argc != 4) {
        printf ("Usage: ./app TIMER_INTERVAL[1-100] TIMER_CNT[1-100] TIMER_INIT[0001-8000]\n");
        return -1;
    }

	my_data.interval = atoi(argv[1]);
	my_data.cnt = atoi(argv[2]);
	
	// ASCII to Integer
	for (i=0; i<4; i++) {
		my_data.init[i] = argv[3][i] - 0x30;
		
		if(my_data.init[i] != 0)
			count++;
	}

	// If it has more than 1 non-zero digits
	if (count != 1) {
		printf ("How to use :For TIMER_INIT, ONLY '1' digit can be in [1-8].\n");
		printf ("            Otherwise, others must be 0! \n");
		return -1;
	}
	

	// Open Device
    fd = open (ESSW_DEVICE_NAME, O_RDWR);

	// Initialize Device
    ioctl (fd, ESSW_DEVICE_SET, &my_data);

	// Start Timer
    ioctl (fd, ESSW_DEVICE_OPERATE, &my_data);

	// Close Device
    close (fd);

    return 0;
}