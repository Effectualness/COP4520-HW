#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

typedef struct ret
{
    int num;
    struct ret* next;
    struct ret* prev;
}ret;

//globals
ret* globalPrimeHead;
ret* globalPrimeTail;
int sum;
HANDLE* sem;

//functions
DWORD PrimeThread(void* numLimit);
int PrimeChecker(int num);

int main()
{
    //Initalize
    int numThreads = 8;
    int numLimit = 100000000;
    double sum = 17;
    int i;
    DWORD threadRet;
    void** arg;
    clock_t time;
    double totalTime;
    HANDLE* thread = (HANDLE*)malloc(sizeof(HANDLE)*numThreads);
    sem = CreateSemaphoreA(NULL, 1, 1, NULL);
    globalPrimeHead = (ret*)malloc(sizeof(ret));
    globalPrimeTail = globalPrimeHead;
    globalPrimeTail->prev = NULL;
    globalPrimeTail->num = 0;
    for(i=0; i<9; i++)
    {
        globalPrimeTail->next = (ret*)malloc(sizeof(ret));
        globalPrimeTail = globalPrimeTail->next;
        globalPrimeTail->prev = NULL;
        globalPrimeTail->num = 0;
    }
    globalPrimeTail->next = NULL;

    //create threads
    time = clock();
    for(i=0; i<numThreads; i++)
    {
        arg = (void**)malloc(sizeof(void*)*3);
        arg[0] = (int*)i;
        arg[1] = (int*)numThreads;
        arg[2] = (int*)numLimit;
        thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrimeThread, (void*)arg, 0, NULL);
    }

    //Wait for threads to finish and get return values for total sum
    for(i=0; i<numThreads; i++)
    {
        WaitForSingleObject(thread[i], INFINITE);
        GetExitCodeThread(thread[i], &threadRet);
        sum += threadRet;
    }
    time = clock() - time;
    totalTime = ((double)(time))/CLOCKS_PER_SEC;

    //free all allocated data and handles
    CloseHandle(sem);
    for(i=0; i<numThreads; i++)
    {
        CloseHandle(thread[i]);
    }
    free(thread);
    printf("time %.2f\n", totalTime);

    printf("sum %.lf\n", sum); //Used during verification process
}

DWORD PrimeThread(void* arg)
{
    void** arr = (void**)arg;
    int threadNum = (int)arr[0] * 2 + 11;
    int numThreads = (int)arr[1] * 2;
    int numLimit = (int)arr[2];
    free(arg);
    int sum = 0;
    int i, j;
    //Loop starts at 11 for first thread, plus two for each additional thread (13 for thread two, 15 for three, etc.)
    //Increments by double the number of threads, so each thread has a unique odd number
    //Inner loop check all odd numbers greater than 1 and less than/equal to sqrt i. If i is evenly divisible, not prime
    for(i=threadNum; i<numLimit; i=i+numThreads) 
    {
        for(j=3; j*j<=i; j=j+2) 
        {
            if(i%j == 0)
            {
                break;
            }
        }
        if(j*j>i)
        {
            sum += i;
        }
    }
    return sum;
}

int PrimeChecker(int num)
{
    int i;
    
}