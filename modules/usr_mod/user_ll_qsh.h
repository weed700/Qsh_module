#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <linux/fs.h>
#include <linux/quota.h>
#include <linux/dqblk_xfs.h>

#include <sys/ioctl.h>
#include <sys/quota.h> 
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <dirent.h>

#define DEVICE_NAME "/dev/qshdrv"

#ifndef PRJQUOTA
#define PRJQUOTA    2
#endif
#ifndef Q_XGETPQUOTA
#define Q_XGETPQUOTA QCMD(Q_XGETQUOTA, PRJQUOTA)
#endif

#include <stdio.h>
#include <stdlib.h>

//project quota id and quota use FS path
typedef struct q_data{
    unsigned int id;
    char special[50];
    char con_path[90];
}q_data_t;

/*
//pid를 통해 종료 플래그 변경
typedef struct q_pid_conpath{
    int pid;
    char path[120];
}q_pid_path;
*/

//linkedlist struct
typedef struct tagNode{
    q_data_t Data;
    struct tagNode* NextNode;
}Node;



/*함수 원형*/
Node* Qsh_CreateNode(q_data_t NewData);             //노드 생성

void Qsh_DestroyNode(Node* Node);                   //노드 소멸
void Qsh_AppendNode(Node** Head, Node* NewNode);    //노드 추가
//void Qsh_InsertAfter(Node* Current, Node* NewNode); //노드 삽입 
void Qsh_InsertNewHead(Node** Head, Node* NewNode);
void Qsh_RemoveNode(Node** Head, Node* Remove);     //노드 제거
Node* Qsh_GetNodeAt(Node* Head, int Location);       //노드 탐색
int Qsh_GetNodeCount(Node* Head);                   //노드 수 세기

