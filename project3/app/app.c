#include "app.h"

int main(int argc, char* argv[])
{
    int fd,i;
    char buf[2]={0,};
    
    fd=open(SW_NAME,O_RDWR);

    write(fd,buf,2);

    close(fd);

    return 0;
}