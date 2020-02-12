#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define SPARE_DISK "/dev/sdb"

int dev;

void test()
{
    system("ls");
}

int main()
{

    char* shellex;                              //실행 시킬 명령어
    char shellpath[30]="/root/init_sh_qsh.sh";  //script가 생성될 위치
    int i;

    char sc[][100]={
        {"#!/bin/bash\n"},
        {"path=$1\n"},
        {"(echo n; echo e; echo \"\"; echo \"\"; echo \"\"; echo w;) | fdisk $path\n"},
    };  

    FILE* fp; 

    fp = fopen(shellpath,"w");
    if(NULL == fp) 
        return -1; 

    for(i = 0;i<sizeof(sc)/sizeof(sc[0]);i++)
        fwrite(sc[i],1,strlen(sc[i]),fp);

    fclose(fp);

    if(-1 == chmod(shellpath,0755))
        return -1; 

    shellex = (char*)malloc(sizeof(char)*50); 

    sprintf(shellex,"%s %s",shellpath,SPARE_DISK);    
    system(shellex);
    printf("\n");

    dev=open("/dev/sdb", O_RDWR);
    

    
    test();
    remove(shellpath);
    free(shellex);

    return 0;

}
