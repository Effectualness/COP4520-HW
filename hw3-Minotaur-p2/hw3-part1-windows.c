#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

//These values may be changed
int print = 0;
int numPresents = 500000;
int numServants = 4;

typedef struct Ordered
{
    int num;
    HANDLE mutex;
    struct Ordered *next;
}Ordered;

int *unsortedPresents;
int unsortedHead;
Ordered *sortedList;
HANDLE sortedLock, unsortedLock;

void AddPresent(int num, int id)
{
    Ordered *node = (Ordered*)malloc(sizeof(Ordered));
    node->num = num;
    node->next = NULL;
    node->mutex = CreateMutexA(NULL, FALSE, NULL);
    Ordered *pred, *curr;
    WaitForSingleObject(sortedLock, INFINITE);
    pred = sortedList;
    if(pred == NULL)
    {
        sortedList = node;
        if(print)
        {
            printf("Servant %d added new head %d\n", id, num);
        }
        ReleaseSemaphore(sortedLock, 1, NULL);
        return;
    }
    WaitForSingleObject(pred->mutex, INFINITE);
    if(num <= pred->num)
    {
        sortedList = node;
        sortedList->next = pred;
        if(print)
        {
            printf("Servant %d replaced head %d with new head %d\n", id, pred->num, num);
        }
        ReleaseSemaphore(sortedLock, 1, NULL);
        ReleaseMutex(pred->mutex);
        return;
    }
    ReleaseSemaphore(sortedLock, 1, NULL);
    while(1)
    {
        curr = pred->next;
        if(curr == NULL)
        {
            pred->next = node;
            if(print)
            {
                printf("Servant %d replaced tail %d with new tail %d\n", id, pred->num, num);
            }
            ReleaseMutex(pred->mutex);
            return;
        }
        if(num <= curr->num)
        {
            node->next = curr;
            pred->next = node;
            if(print)
            {
                printf("Servant %d inserted between nodes %d and %d with new node %d\n", id, pred->num, curr->num, num);
            }
            ReleaseMutex(pred->mutex);
            return;
        }
        WaitForSingleObject(curr->mutex, INFINITE);
        ReleaseMutex(pred->mutex);
        pred = curr;
    }
}

int RemovePresent(int id)
{
    Ordered *pred;
    WaitForSingleObject(sortedLock, INFINITE);
    pred = sortedList;
    if(pred == NULL)
    {
        if(print)
        {
            printf("Servant %d attempted to remove a present, but the list was empty.\n", id);
        }
        ReleaseSemaphore(sortedLock, 1, NULL);
        return 0;
    }
    WaitForSingleObject(pred->mutex, INFINITE);
    sortedList = pred->next;
    if(print)
    {
        printf("Servant %d released old head %d\n", id, pred->num);
    }
    ReleaseSemaphore(sortedLock, 1, NULL);
    CloseHandle(pred->mutex);
    free(pred);
    return 1;
}

int ContainsPresent(int num, int id)
{
    Ordered *pred, *curr;
    WaitForSingleObject(sortedLock, INFINITE);
    pred = sortedList;
    if(pred == NULL)
    {
        if(print)
        {
            printf("Servant %d attempted to search for node %d, but the list was empty.\n", id, num);
        }
        ReleaseSemaphore(sortedLock, 1, NULL);
        return 0;
    }
    WaitForSingleObject(pred->mutex, INFINITE);
    if(num == pred->num)
    {
        if(print)
        {
            printf("Servant %d searched for and found at the head %d the requested node %d\n", id, pred->num, num);
        }
        ReleaseSemaphore(sortedLock, 1, NULL);
        ReleaseMutex(pred->mutex);
        return 1;
    }
    ReleaseSemaphore(sortedLock, 1, NULL);
    while(1)
    {
        curr = pred->next;
        if(curr == NULL)
        {
            if(print)
            {
                printf("Servant %d seached for but could not find node %d\n", id, num);
            }
            ReleaseMutex(pred->mutex);
            return 0;
        }
        if(num == curr->num)
        {
            if(print)
            {
                printf("Servant %d searched for and found between nodes %d and %d the requested node %d\n", id, pred->num, curr->num, num);
            }
            ReleaseMutex(pred->mutex);
            return 1;
        }
        WaitForSingleObject(curr->mutex, INFINITE);
        ReleaseMutex(pred->mutex);
        pred = curr;
    }
}

DWORD Servant(void* param)
{
    srand(GetCurrentThreadId());
    int randNum, id = (int)param + 1, num;
    int *data = (int*)calloc(4, sizeof(int));
    while(unsortedHead < numPresents)
    {
        randNum = rand() % 3;
        switch(randNum)
        {
            case 0: //add to chain (unsorted bag)
            {
                WaitForSingleObject(unsortedLock, INFINITE);
                if(unsortedHead >= numPresents)
                {
                    ReleaseSemaphore(unsortedLock, 1, NULL);
                    break;
                }
                num = unsortedPresents[unsortedHead++];
                ReleaseSemaphore(unsortedLock, 1, NULL);
                AddPresent(num, id);
                data[0] = data[0] + 1;
                break;
            }
            case 1: //remove from chain (thank you card)
            {
                data[1] = data[1] + RemovePresent(id);
                break;
            }
            case 2: //Check for element through flag (minotaurs request)
            {
                randNum = rand();
                randNum << 16;
                randNum = randNum | rand();
                randNum = randNum % numPresents;
                randNum = ContainsPresent(randNum, id);
                if(randNum == 1)
                {
                    data[2] = data[2] + 1;
                }
                else
                {
                    data[3] = data[3] + 1;
                }
                break;
            }
            default:
            {
                printf("An error has occurred. Exiting...\n");
                exit(0);
            }
        }
    }
    while(sortedList != NULL)
    {
        data[1] = data[1] + RemovePresent(id);
    }
    return (DWORD)data;
}

int main()
{
    int i, randNum, tempNum;
    int *data;
    PDWORD tempData;
    unsortedHead = 0;
    sortedList = NULL;
    srand(GetCurrentThreadId());
    sortedLock = CreateSemaphoreA(NULL, 1, 1, NULL);
    unsortedLock = CreateSemaphoreA(NULL, 1, 1, NULL);
    unsortedPresents = (int*)malloc(sizeof(int) * numPresents);
    for(i=0; i<numPresents; i++)
    {
        unsortedPresents[i] = i;
    }
    for(i=0; i<numPresents; i++)
    {
        randNum = rand();
        randNum << 16;
        randNum = randNum | rand();
        randNum = randNum % numPresents;
        tempNum = unsortedPresents[i];
        unsortedPresents[i] = unsortedPresents[randNum];
        unsortedPresents[randNum] = tempNum;
    }
    HANDLE* threadArray = (HANDLE*)malloc(sizeof(HANDLE) * numServants);
    printf("Servants are beginning, please wait...\n");
    for(i=0; i<numServants; i++)
    {
        threadArray[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Servant, (int*)i, 0, NULL);
    }
    for(i=0; i<numServants; i++)
    {
        WaitForSingleObject(threadArray[i], INFINITE);
    }
    printf("\n");
    for(i=0; i<numServants; i++)
    {
        GetExitCodeThread(threadArray[i], tempData);
        data = (int*)(*tempData);
        printf("Servant %d added %d presents, wrote %d thank you cards, successfully verified %d presents were already sorted, and could not find %d presents.\n", i+1, data[0], data[1], data[2], data[3]);
        free(data);
        CloseHandle(threadArray[i]);
    }
    free(unsortedPresents);
    free(threadArray);
    CloseHandle(sortedLock);
    CloseHandle(unsortedLock);
    if(sortedList == NULL && unsortedHead == numPresents)
    {
        printf("\nlist was confirmed empty: All presents were sorted and all thank you cards were sent.\n");
    }
    else
    {
        printf("\nList was confirmed NOT empty: An error has occured.\n");
    }
    return 0;
}