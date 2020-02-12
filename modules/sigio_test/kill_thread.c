#include <stdio.h>
#include <pthread.h>

#define THREAD_TIMEOUT_SEC 5

static int check_thread_status(int *pnWorkStatus, int nWaitTime);
static void *exec_loop(void *pArg);

int main(int argc, char **argv)
{
    int nWorkStatus = 0;
    pthread_t stExecThread;

    printf("무한 반복 스레드 실행\n");

//    1. 스레드 생성 및 백그라운드 실행
        pthread_create(&stExecThread, NULL, exec_loop, (void*)&nWorkStatus);
        pthread_detach(stExecThread);

    // 2. 위에서 실행한 스레드가 5초 안에 정상 종료되었는지 확인
    if ( !check_thread_status(&nWorkStatus, THREAD_TIMEOUT_SEC) )
    {
        printf("%d초가 초과 되었기 때문에 exec_thread 종료\n", THREAD_TIMEOUT_SEC);

        // 3. 주어진 시간안에 종료되지 않았기 때문에 해당 스레드가 종료될 수 있도록 함
        pthread_cancel(stExecThread);
    }

    printf("Bye!\n");
    getchar();

    return 0;
}

// 무한 반복용 테스트 스레드 함수
static void *exec_loop(void *pArg)
{
    int i = 0;
    int *pnWorkStatus = (int*)pArg;

    while ( 1 )
    {
//        printf("%s() %d초\n", __func__, ++i);
        sleep(1);
    }

    // 완료됨을 기록 (0에서 1로)
    *pnWorkStatus = 1;

    return NULL;
}

// nWaitTime(초) 동안 pnWorkStatus이 변하는지 확인하는 함수
static int check_thread_status(int *pnWorkStatus, int nWaitTime)
{
    int i;

    // 주어진 nWaitTime 만큼만 대기
    for ( i = 0; i < nWaitTime; i++ )
    {
        // 스레드가 완료된 시점에서는 *pnWorkStatus는 1이 된다.
        if ( *pnWorkStatus )
            return 1;
        sleep(1);
    }

    return 0;
}

