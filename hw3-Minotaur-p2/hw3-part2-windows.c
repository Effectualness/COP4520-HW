#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

//These variables may be changed
int numSensors = 8;
int numHours = 1;
int timerMicroseconds = 1000; //Set to 60000000 for readings once per minute
int print = 0;

HANDLE timer, sem;

DWORD Sensor(void *param)
{
    srand(GetCurrentThreadId());
    int *data = (int*)param;
    int i;
    for(i=0; i<numHours*60; i++)
    {
        *data = rand() % 171 - 100;
        WaitForSingleObject(sem, INFINITE);
        WaitForSingleObject(timer, INFINITE);
    }
}

typedef struct Hour
{
    int largestTemp[5];
    int smallestTemp[5];
    int largestDifference;
    int periodNum;
    struct Hour *next;
}Hour;

int main()
{
    int i, j, k, l, sFlag, lFlag, sinterval = 1000, linterval = -1000, periodNum;
    int sensorData[numSensors];
    Hour *dataHead, *data;
    LARGE_INTEGER dueTime;
    dueTime.QuadPart = -10 * timerMicroseconds;
    sem = CreateSemaphoreA(NULL, 0, numSensors, NULL);
    printf("Threads are beginning, please wait...\n");
    HANDLE* threadArray = (HANDLE*)malloc(sizeof(HANDLE) * numSensors);
    for(i=0; i<numSensors; i++)
    {
        sensorData[i] = -1000;
        threadArray[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Sensor, sensorData+i, 0, NULL);
    }
    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    //Loop for each hour
    for(k=0; k<numHours; k++)
    {
        if(k != 0)
        {
            data->next = (Hour*)malloc(sizeof(Hour));
            data = data->next;
        }
        else
        {
            data = (Hour*)malloc(sizeof(Hour));
            dataHead = data;
        }
        for(i=0; i<5; i++)
        {
            data->largestTemp[i] = -1000;
            data->smallestTemp[i] = 1000;
        }
        data->next = NULL;
        data->largestDifference = -1000;
        //loop for each minute within hour
        for(j=0; j<60; j++)
        {
            if(print)
            {
                printf("\nTemp random Values:");
            }
            SetWaitableTimer(timer, &dueTime, 0, NULL, NULL, 0);
            for(i=0; i<numSensors; i++)
            {
                ReleaseSemaphore(sem, 1, NULL);
            }
            //Loop for each sensor
            for(i=0; i<numSensors; i++)
            {
                while(sensorData[i] == -1000);
                if(sensorData[i] < sinterval)
                {
                    sinterval = sensorData[i];
                }
                if(sensorData[i] > linterval)
                {
                    linterval = sensorData[i];
                }
                if(print)
                {
                    printf(" %d", sensorData[i]);
                }
                sFlag = 0;
                lFlag = 0;
                //loop for each small and large array
                for(l = 0; l < 5; l++)
                {
                    if(sensorData[i] >= data->largestTemp[l] && lFlag == 0)
                    {
                        //Shift array if needed
                        for(lFlag = 4; lFlag > l; lFlag--)
                        {
                            data->largestTemp[lFlag] = data->largestTemp[lFlag-1];
                        }
                        data->largestTemp[l] = sensorData[i];
                        lFlag = 1;
                    }
                    if(sensorData[i] <= data->smallestTemp[l] && sFlag == 0)
                    {
                        //Shift array if needed
                        for(sFlag = 4; sFlag > l; sFlag--)
                        {
                            data->smallestTemp[sFlag] = data->smallestTemp[sFlag-1];
                        }
                        data->smallestTemp[l] = sensorData[i];
                        sFlag = 1;
                    }
                }
                sensorData[i] = -1000;
            }
            if(j % 10 == 9)
            {
                if(linterval - sinterval > data->largestDifference)
                {
                    data->largestDifference = linterval - sinterval;
                    data->periodNum = j / 10 + 1;
                }
                if(print)
                {
                    printf("\nTemp large interval %d and small interval %d with difference %d for period %d", linterval, sinterval, linterval-sinterval, j/10+1);
                }
                sinterval = 1000;
                linterval = -1000;
            }
            if(WaitForSingleObject(timer, 0) == WAIT_OBJECT_0)
            {
                printf("\nMain has missed a timer interval. Exiting...\n");
                exit(0);
            }
            if(print)
            {
                printf("\nsuccess loop %d\nSaved largest values:", j);
                for(i=0; i<5; i++)
                {
                    printf(" %d", data->largestTemp[i]);
                }
                printf("\nSaved smallest values:");
                for(i=0; i<5; i++)
                {
                    printf(" %d", data->smallestTemp[i]);
                }
                printf("\n");
                if(j % 10 == 9)
                {
                    printf("Saved Largest difference: %d\n", data->largestDifference);
                }
            }
            WaitForSingleObject(timer, INFINITE);
        }
    }
    for(i=0; i<numSensors; i++)
    {
        WaitForSingleObject(threadArray[i], INFINITE);
    }
    data = dataHead;
    printf("\nFinal results:\n");
    j = 1;
    while(data != NULL)
    {
        printf("\nHour %d\nLargest values:", j++);
        for(i=0; i<5; i++)
        {
            printf(" %d", data->largestTemp[i]);
        }
        printf("\nSmallest values:");
        for(i=0; i<5; i++)
        {
            printf(" %d", data->smallestTemp[i]);
        }
        printf("\nLargest difference: %d at 10-minute period #%d\n", data->largestDifference, data->periodNum);
        dataHead = data->next;
        free(data);
        data = dataHead;
    }
    free(threadArray);
    CloseHandle(timer);
    CloseHandle(sem);
    return 0;
}