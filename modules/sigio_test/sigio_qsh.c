#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fs.h>

//quota & xfs
#include <linux/dqblk_xfs.h>
#include <linux/quota.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>

typedef struct q_data{
        unsigned int id;
            char special[100];
}q_data_t;


void sigio_handler(int signo)
{
    q_data_t d;
    int fd;

    fd = open("/dev/qshdrv",O_RDWR);

    printf("sigio_test ... \n");
    read(fd,&d,sizeof(d));

    printf("tset : %u %s\n", d.id, d.special);
     
    close(fd);
}


void main()
{
    struct sigaction sigact, oldact;
    int oflag;
    int dev;

    sigact.sa_handler = sigio_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_INTERRUPT;

    if(sigaction(SIGIO, &sigact, &oldact) < 0)
    {
        perror("sigaction error:");
        exit(0);
    }

    dev = open("/dev/qshdrv",O_RDWR);
   
    if(0 <= dev)
    {
        fcntl(dev, F_SETOWN,getpid());
        oflag = fcntl(dev, F_GETFL);
        fcntl(dev,F_SETFL, oflag | FASYNC);
        while(1)
            sleep(1);
        close(dev);
    }
}
