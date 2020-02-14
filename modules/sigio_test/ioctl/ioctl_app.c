#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char **argv){
	int dev;
    int i;

	dev= open("/dev/kw_device",O_RDWR);

	if(dev<0)
	{
	printf("device file open error\n");
	return -1;
	}
	//ioctl(dev,1);
	ioctl(dev,2);
	//ioctl(dev,3);
	close(dev);
	return 0;
}
