#include "user_ll_qsh.h"

#define SIG_TEST 44
#define SPARE_DISK "/dev/nvme2n1"

Node* List = NULL;          //리스트 헤더
Node* Current = NULL;       //리스트 탐색할 때 필요
Node* NewNode = NULL;       //새로운 노드

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t thread_cond = PTHREAD_COND_INITIALIZER;
pthread_t thread;

int dev;                    //장치 디스크립터 
int Count = 0;              //노드 개수

void thread_clean_up(void* arg)
{
    int i;
    
    syslog(LOG_INFO | LOG_LOCAL0,"thread_clean_up in!\n");
       
    //모든 노드 제거
    for(i = 0; i<Count;i++)
    {
        Current = Qsh_GetNodeAt(List,0);

        if(NULL != Current)
        {
            Qsh_RemoveNode(&List, Current);
            Qsh_DestroyNode(Current);
        }
    }
    close(dev);
}

static void signal_cleanup(int n, siginfo_t* info, void* unused)
{
    syslog(LOG_INFO | LOG_LOCAL0,"sigio_kill!!\n");
    pthread_cancel(thread); //thread 종료  
}

int qsh_script_exec(unsigned long long size, char* c_path)
{
    char* shellex;                              //실행 시킬 명령어
    char shellpath[30]="/root/exec_sh_qsh.sh";  //script가 생성될 위치
    char temp[10];
    int i;

    char sc[16][150]={
        {"#!/bin/bash\n"},
        {"dir=\"/root/qsh_host_\"\n"},
        {"path=$1\n"},
        {"size=$2\n"},
        {"size='+'$size\n"},
        {"(echo n; echo l; echo \"\"; echo $size; echo w;) | fdisk $path\n"},
        {"partprobe\n"},
        {"path=`fdisk -l $path | tail -n 1 | cut -d ' ' -f1`\n"},
        {"path_sl=${path:5}\n"},
        {"dir=$dir$path_sl\n"},
        {"mkdir $dir\n"},
        {"chmod 755 $dir\n"},
        {"(echo y;) | mkfs.xfs -f $path\n"},
        {"mount $path $dir\n"},
    };  

    char* base_dir = "/merged/root/qsh_bak_dir";
    char* sh_tmp = (char*)malloc(sizeof(char)*150);
    char mkdir_tmp[150];
    FILE* fp; 

    sprintf(mkdir_tmp,"mkdir %s%s\n",c_path,base_dir);
    strcpy(sc[14],mkdir_tmp);

    sprintf(sh_tmp,"mount --bind $dir %s%s",c_path,base_dir);  
    strcpy(sc[15],sh_tmp);

    fp = fopen(shellpath,"w");
    if(NULL == fp) 
        return -1; 
    
    for(i = 0;i<sizeof(sc)/sizeof(sc[0]);i++)
        fwrite(sc[i],1,strlen(sc[i]),fp);
 
    fclose(fp);

    if(-1 == chmod(shellpath,0755))
        return -1;

    size = (((size*512)/1024)/1024)*10;
    sprintf(temp,"%lluM",size);

    shellex = (char*)malloc(sizeof(char)*50); 

    sprintf(shellex,"%s %s %s",shellpath,SPARE_DISK,temp);
    syslog(LOG_INFO | LOG_LOCAL0,"qsh_script_exec : %s\n",shellex);
  
    system(shellex);
    
    remove(shellpath);
    free(sh_tmp);
    free(shellex);

    return 0;

}

void sigio_handler(int signo)
{
    syslog(LOG_INFO | LOG_LOCAL0,"sigio_handler!!\n");
    
    q_data_t d;
    int fd;
    fs_disk_quota_t temp_d;

    fd = open(DEVICE_NAME, O_RDONLY);
    
    syslog(LOG_INFO | LOG_LOCAL0,"sigio_handler in!\n");

    read(fd,&d,sizeof(d));

    syslog(LOG_INFO | LOG_LOCAL0,"test : %u %s %s\n",d.id, d.special,d.con_path);


    //새로운 노드 생성
    NewNode = Qsh_CreateNode(d);
    Qsh_AppendNode(&List, NewNode);

    //script 실행(예비 디스크 파티션 나누기 및 host 디렉토리 mount)
    quotactl(Q_XGETPQUOTA,NewNode->Data.special,NewNode->Data.id,&temp_d); //생성된 컨테이너의 디스크 용량 받아오기
    syslog(LOG_INFO | LOG_LOCAL0,"disk size : %u : %llu MB!!!!!!\n",temp_d.d_id, ((temp_d.d_blk_hardlimit*512)/1024)/1024);

    qsh_script_exec(temp_d.d_blk_hardlimit, d.con_path);
    close(fd);
}

int init_script()
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

    remove(shellpath);
    free(shellex);

    return 0;
}


void* main_thread(void* arg)
{
    //sigio
    struct sigaction sigact,sigkill, oldact;
    int oflag;
//    int flag = 0;

    int i=0;
    fs_disk_quota_t d;

    pid_t pid;
    
    syslog(LOG_INFO | LOG_LOCAL0,"main_thread in! \n");

    //쓰레드에 대한 취소요청을 받아드린다.
    //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    //clean handler 함수를 등록 ( 여기서는 thread_clean_up 함수)
    pthread_cleanup_push(thread_clean_up, (void*)NULL);
   
    //시그널 핸들러 생성
    sigact.sa_handler = sigio_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;

    //시그널 핸들러 생성(스레드 종료 시점)
    sigkill.sa_sigaction = signal_cleanup;
    sigkill.sa_flags = SA_SIGINFO;

    if(sigaction(SIGIO, &sigact, NULL ) < 0)
    {
        perror("sigaction error!\n");
        exit(0);
    }
    
    if(sigaction(SIG_TEST, &sigkill, NULL) < 0)
    {
        perror("sigaction error!\n");
        exit(0);
    }

    //init_script
    if(-1 == init_script())
        syslog(LOG_INFO | LOG_LOCAL0,"init_script() erorr!\n"); 
    

    //fcntl 파일 디스크립트
    dev = open(DEVICE_NAME, O_RDWR);
   
    if(0 <= dev)
    {
        pid = getpid();
        write(dev,&pid,sizeof(pid));
        //SIGIO signal을수신하도록 설정
        fcntl(dev, F_SETOWN, pid);
        oflag = fcntl(dev,F_GETFL);
        //file descriptor를 비동기로 설정
        fcntl(dev,F_SETFL, oflag | FASYNC);
    }

     while(1)
     {
        if(NULL != List)
        {            
            /* 노드 찾아서 출력*/
            Count = Qsh_GetNodeCount(List);
            printf("--- ProjID    block_count    UsedDisk(KB)    Con_total_size(MB) ---\n");
            printf("-------------------------------------------------------------------\n");
            for(i=0;i < Count;i++)
            {
                Current = Qsh_GetNodeAt(List,i);
                quotactl(Q_XGETPQUOTA,Current->Data.special,Current->Data.id,&d);
                syslog(LOG_INFO | LOG_LOCAL0,"ID : %u,  block_count : %llu ,UsedDisk: %llu KB, ConSize : %llu\n",d.d_id, d.d_bcount ,(d.d_bcount*512)/1024, ((d.d_blk_hardlimit*512)/1024)/1024);
//                printf("--- ProjID    block_count    UsedDisk(KB)    Con_total_size(MB) ---\n");
                printf("     %u        %llu         %llu                   %llu\n",d.d_id, d.d_bcount, (d.d_bcount*512)/1024, ((d.d_blk_hardlimit*512)/1024)/1024);
            }
        }
        sleep(10);
     }

    //cleanup handler 해제
    pthread_cleanup_pop(0);
}


int main(int argc, char** argv)
{
    int thr_id;
    int status;
 
    syslog(LOG_INFO | LOG_LOCAL0,"module -> user info\n");

    //스레드 생성
    thr_id = pthread_create(&thread,NULL,main_thread,NULL);

    if(thr_id < 0){
        syslog(LOG_INFO | LOG_LOCAL0,"thread create error!\n");
        exit(0);
    }
 
    //스레드 종료를 기다림.
    pthread_join(thread, (void **)&status);
    

    return 0;
}

