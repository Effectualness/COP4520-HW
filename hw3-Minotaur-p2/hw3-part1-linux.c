#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

//These values may be changed
int print = 0;
int numPresents = 500000;
int numServants = 4;

typedef struct Ordered
{
    int num;
    pthread_mutex_t *mutex;
    struct Ordered *next;
}Ordered;

int *unsortedPresents;
int unsortedHead;
Ordered *sortedList;
sem_t *sortedLock, *unsortedLock;

void AddPresent(int num, int id)
{
    Ordered *node = (Ordered*)malloc(sizeof(Ordered));
    node->num = num;
    node->next = NULL;
    node->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(node->mutex, NULL);
    Ordered *pred, *curr;
    sem_wait(sortedLock);
    pred = sortedList;
    if(pred == NULL)
    {
        sortedList = node;
        if(print)
        {
            printf("Servant %d added new head %d\n", id, num);
        }
        sem_post(sortedLock);
        return;
    }
    pthread_mutex_lock(pred->mutex);
    if(num <= pred->num)
    {
        sortedList = node;
        sortedList->next = pred;
        if(print)
        {
            printf("Servant %d replaced head %d with new head %d\n", id, pred->num, num);
        }
        sem_post(sortedLock);
        pthread_mutex_unlock(pred->mutex);
        return;
    }
    sem_post(sortedLock);
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
            pthread_mutex_unlock(pred->mutex);
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
            pthread_mutex_unlock(pred->mutex);
            return;
        }
        pthread_mutex_lock(curr->mutex);
        pthread_mutex_unlock(pred->mutex);
        pred = curr;
    }
}

int RemovePresent(int id)
{
    Ordered *pred;
    sem_wait(sortedLock);
    pred = sortedList;
    if(pred == NULL)
    {
        if(print)
        {
            printf("Servant %d attempted to remove a present, but the list was empty.\n", id);
        }
        sem_post(sortedLock);
        return 0;
    }
    pthread_mutex_lock(pred->mutex);
    sortedList = pred->next;
    if(print)
    {
        printf("Servant %d released old head %d\n", id, pred->num);
    }
    sem_post(sortedLock);
    pthread_mutex_destroy(pred->mutex);
    free(pred->mutex);
    free(pred);
    return 1;
}

int ContainsPresent(int num, int id)
{
    Ordered *pred, *curr;
    sem_wait(sortedLock);
    pred = sortedList;
    if(pred == NULL)
    {
        if(print)
        {
            printf("Servant %d attempted to search for node %d, but the list was empty.\n", id, num);
        }
        sem_post(sortedLock);
        return 0;
    }
    pthread_mutex_lock(pred->mutex);
    if(num == pred->num)
    {
        if(print)
        {
            printf("Servant %d searched for and found at the head %d the requested node %d\n", id, pred->num, num);
        }
        sem_post(sortedLock);
        pthread_mutex_unlock(pred->mutex);
        return 1;
    }
    sem_post(sortedLock);
    while(1)
    {
        curr = pred->next;
        if(curr == NULL)
        {
            if(print)
            {
                printf("Servant %d seached for but could not find node %d\n", id, num);
            }
            pthread_mutex_unlock(pred->mutex);
            return 0;
        }
        if(num == curr->num)
        {
            if(print)
            {
                printf("Servant %d searched for and found between nodes %d and %d the requested node %d\n", id, pred->num, curr->num, num);
            }
            pthread_mutex_unlock(pred->mutex);
            return 1;
        }
        pthread_mutex_lock(curr->mutex);
        pthread_mutex_unlock(pred->mutex);
        pred = curr;
    }
}

void* Servant(void* param)
{
    srand(pthread_self());
    int randNum, id = (int*)param, num;
    id = id + 1;
    int *data = (int*)calloc(4, sizeof(int));
    while(unsortedHead < numPresents)
    {
        randNum = rand() % 3;
        switch(randNum)
        {
            case 0: //add to chain (unsorted bag)
            {
                sem_wait(unsortedLock);
                if(unsortedHead >= numPresents)
                {
                    sem_post(unsortedLock);
                    break;
                }
                num = unsortedPresents[unsortedHead++];
                sem_post(unsortedLock);
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
    return (void*)data;
}

int main()
{
    int i, randNum, tempNum;
    void *retData;
    int **data;
    unsortedHead = 0;
    sortedList = NULL;
    srand(pthread_self());
    sortedLock = (sem_t*)malloc(sizeof(sem_t));
    unsortedLock = (sem_t*)malloc(sizeof(sem_t));
    sem_init(sortedLock, 0, 1);
    sem_init(unsortedLock, 0, 1);
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
    pthread_t* threadArray = (pthread_t*)malloc(sizeof(pthread_t) * numServants);
    printf("Servants are beginning, please wait...\n");
    for(i=0; i<numServants; i++)
    {
        pthread_create(&threadArray[i], NULL, Servant, (void*)(__intptr_t)i);
    }
    for(i=0; i<numServants; i++)
    {
        pthread_join(threadArray[i], &retData);
        data[i] = (int*)retData;
    }
    printf("\n");
    for(i=0; i<numServants; i++)
    {
        printf("Servant %d added %d presents, wrote %d thank you cards, successfully verified %d presents were already sorted, and could not find %d presents.\n", i+1, data[i][0], data[i][1], data[i][2], data[i][3]);
        free(data[i]);
    }
    free(unsortedPresents);
    free(threadArray);
    sem_destroy(sortedLock);
    sem_destroy(unsortedLock);
    free(unsortedLock);
    free(sortedLock);
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