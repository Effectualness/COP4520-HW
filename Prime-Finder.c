#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

typedef struct retStruct
{
    double sum;
    double total;
    int* top;
    struct retStruct* next;
}retStruct;

//functions
DWORD* PrimeThread(void* numLimit);
int* arraySort(void** arg, int numThreads);
void merge(int *arr, int start, int mid, int end);
void mergeSort(int *arr, int start, int end);

int main()
{
    //user values (Can be changed)
    int numThreads = 8;
    int numLimit = 100000000;

    //initialize
    if(numLimit<100)
    {
        printf("The mimimum number limit for this program is 100.\nContinuing with 100...\n");
        numLimit = 100;
    }
    double sum = 17;
    double total = 0;
    int i;
    int* arr;
    void** arg;
    clock_t time;
    HANDLE* thread = (HANDLE*)malloc(sizeof(HANDLE)*numThreads);
    retStruct* retHead = (retStruct*)malloc(sizeof(retStruct));
    retStruct* retTail = retHead;
    retHead->sum = 0;
    retHead->total = 0;
    retHead->top = (int*)calloc(10, sizeof(int));
    for(i=0; i<9; i++)
    {
        retTail->next = (retStruct*)malloc(sizeof(retStruct));
        retTail = retTail->next;
        retTail->sum = 0;
        retTail->total = 0;
        retTail->top = (int*)calloc(10, sizeof(int));
    }
    retTail->next = NULL;

    //create threads in suspended state
    retTail = retHead;
    for(i=0; i<numThreads; i++)
    {
        arg = (void**)malloc(sizeof(void*)*4);
        arg[0] = (int*)i;
        arg[1] = (int*)numThreads;
        arg[2] = (int*)numLimit;
        arg[3] = retTail;
        thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrimeThread, (void*)arg, CREATE_SUSPENDED, NULL);
        retTail = retTail->next;
    }

    //Start threads and clock
    printf("Threads are beginning, please wait...\n");
    time = clock();
    for(i=0; i<numThreads; i++)
    {
        ResumeThread(thread[i]);
    }

    //Wait for threads to finish and end clock
    for(i=0; i<numThreads; i++)
    {
        WaitForSingleObject(thread[i], INFINITE);
    }
    time = clock() - time;

    //Combine each threads data into one
    retTail = retHead;
    arg = (void**)malloc(sizeof(void*)*numThreads);
    for(i=0; i<numThreads; i++)
    {
        sum = sum + retTail->sum;
        total = total + retTail->total;
        arg[i] = retTail->top;
        retTail = retTail->next;
    }
    arr = arraySort(arg, numThreads);

    //write values to file
    FILE* ifp = fopen("primes.txt", "w");
    fprintf(ifp, "Execution time: %.3lf seconds\n", ((double)time)/CLOCKS_PER_SEC);
    fprintf(ifp, "Total number of primes found: %.lf\n", total);
    fprintf(ifp, "Sum of all primes: %.lf\n", sum);
    fprintf(ifp, "Top ten maximum primes, listed in order from lowest to highest:\n");
    for(i=0; i<10; i++)
    {
        fprintf(ifp, "%d ", arr[i]);
    }
    fclose(ifp);

    //free all allocated data and handles
    for(i=0; i<numThreads; i++)
    {
        CloseHandle(thread[i]);
        retTail = retHead->next;
        free(retHead->top);
        free(retHead);
        retHead = retTail;

    }
    free(thread);
    free(arr);
    free(arg);

    printf("Task completed successfully: Data printed to file primes.txt");
    return 1;
}

//Thread to find prime numbers
DWORD* PrimeThread(void* arg)
{
    void** arr = (void**)arg;
    int threadNum = (int)arr[0] * 2 + 11;
    int numThreads = (int)arr[1] * 2;
    int numLimit = (int)arr[2];
    retStruct* ret = (retStruct*)arr[3];
    free(arg);
    int i, j, k=0;
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
            ret->sum += i;
            ret->total++;
            ret->top[k] = i;
            k = (k+1) % 10;
        }
    }
    return (DWORD*)1;
}

int* arraySort(void** arg, int numThreads)
{
    int i;
    int arr[10*numThreads];
    int* tempArr;
    for(i=0; i<numThreads*10; i++)
    {
        if(i%10==0)
        {
            tempArr = (int*)arg[i/10];
        }
        arr[i] = tempArr[i%10];
    }
    mergeSort(arr, 0, 10*numThreads-1);
    tempArr = (int*)malloc(sizeof(int)*10);
    for(i=0; i<10; i++)
    {
        tempArr[i] = arr[10*numThreads-10+i];
    }
    return tempArr;
}

void mergeSort(int *arr, int start, int end) {

	if(start < end) 
    {
		int mid = (start + end) / 2;
		mergeSort(arr, start, mid);
		mergeSort(arr, mid+1, end);
		merge(arr, start, mid, end);
	}
}

void merge(int *arr, int start, int mid, int end) 
{
	int temp[end - start + 1];
	int i = start, j = mid+1, k = 0;
	while(i <= mid && j <= end) 
    {
		if(arr[i] <= arr[j]) 
        {
			temp[k] = arr[i];
			k += 1; i += 1;
		}
		else {
			temp[k] = arr[j];
			k += 1; j += 1;
		}
	}
	while(i <= mid)
    {
		temp[k] = arr[i];
		k += 1; i += 1;
	}
	while(j <= end)
    {
		temp[k] = arr[j];
		k += 1; j += 1;
	}
	for(i = start; i <= end; i += 1)
    {
		arr[i] = temp[i - start];
	}
}