#include "user_ll_qsh.h"

/* 노드 생성 */
Node* Qsh_CreateNode(q_data_t NewData)
{
    Node* NewNode = (Node*)malloc(sizeof(Node));

    NewNode->Data = NewData;
    NewNode->NextNode = NULL;

    return NewNode;
}

/* 노드 소멸 */
void Qsh_DestroyNode(Node* Node)
{
    free(Node);
}

/* 노드 추가 */
void Qsh_AppendNode(Node** Head, Node* NewNode)
{
    if(NULL == (*Head))
        *Head = NewNode;
    else
    {
        Node* Tail = (*Head);

        while(NULL != Tail->NextNode)
        {
            Tail = Tail->NextNode;
        }

        Tail->NextNode = NewNode;
    }
}

//void Qsh_InsertAfter(Node* Current, Node* NewNode)
//{
//    NewNode->NextNode = Current->NextNode;
//    Current->NextNode = NewNode;
//}

/* 새로운 해더 삽입 */
void Qsh_InsertNewHead(Node** Head, Node* NewHead)
{
    if(NULL == *Head)
        (*Head) = NewHead;
    else
    {
        NewHead->NextNode = (*Head);
        (*Head) = NewHead;
    }
}

/* 노드 제거 */
void Qsh_RemoveNode(Node** Head, Node* Remove)
{
    if(Remove == *Head)
        *Head = Remove->NextNode;
    else
    {
        Node* Current = *Head;
        while(NULL != Current && Remove != Current->NextNode)
            Current = Current->NextNode;

        if(NULL != Current)
            Current->NextNode = Remove->NextNode;
    }
}

/* 노드 탐색 */
Node* Qsh_GetNodeAt(Node* Head, int Location)
{
    Node* Current = Head;

    while(NULL != Current && (--Location) >= 0 )
        Current = Current->NextNode;

    return Current;
}

/* 노드 수 세기 */
int Qsh_GetNodeCount(Node* Head)
{
    int Count = 0;
    Node* Current = Head;

    while(NULL != Current)
    {
        Current = Current->NextNode;
        Count++;
    }

    return Count;
}

