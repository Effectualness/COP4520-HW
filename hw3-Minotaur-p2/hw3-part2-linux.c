#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

//These variables may be changed
int numSensors = 8;
int numHours = 1;
int timerMilliseconds = 10; //Set to 60000 for readings once per minute
int print = 0;

sem_t *sem, *semCPU, *semSensor, *timerSem;

void TimerSignal()
{
    sem_post(timerSem);
}

void* Sensor(void *param)
{
    srand(pthread_self());
    int *data = (int*)param;
    int i;
    for(i=0; i<numHours*60; i++)
    {
        sem_wait(sem);
        *data = rand() % 171 - 100;
        sem_post(semCPU);
        sem_wait(semSensor);
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
    timer_t timer = 0;
    sem = (sem_t*)malloc(sizeof(sem_t));
    sem_init(sem, 0, 0);
    semCPU = (sem_t*)malloc(sizeof(sem_t));
    sem_init(semCPU, 0, 0);
    semSensor = (sem_t*)malloc(sizeof(sem_t));
    sem_init(semSensor, 0, 0);
    timerSem = (sem_t*)malloc(sizeof(sem_t));
    sem_init(timerSem, 0, 0);
    printf("Threads are beginning, please wait...\n");
    pthread_t* threadArray = (pthread_t*)malloc(sizeof(pthread_t) * numSensors);
    for(i=0; i<numSensors; i++)
    {
        sensorData[i] = -1000;
        pthread_create(&threadArray[i], NULL, Sensor, sensorData+i);
    }
    struct sigevent sev = { 0 };
    sev.sigev_notify = SIGEV_THREAD;
    sev._sigev_un._sigev_thread._function = TimerSignal;
    struct itimerspec interval = {
        .it_value.tv_sec = timerMilliseconds / 1000,
        .it_value.tv_nsec = timerMilliseconds % 1000 * 1000000,
        .it_interval.tv_sec = timerMilliseconds / 1000,
        .it_interval.tv_nsec = timerMilliseconds % 1000 * 1000000
    };
    timer_create(CLOCK_REALTIME, &sev, &timer);
    timer_settime(timer, 0, &interval, NULL);
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
            for(i=0; i<numSensors; i++)
            {
                sem_post(sem);
            }
            for(i=0; i<numSensors; i++)
            {
                sem_wait(semCPU);
            }
            for(i=0; i<numSensors; i++)
            {
                sem_post(semSensor);
            }
            //Loop for each sensor
            for(i=0; i<numSensors; i++)
            {
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
            if(!sem_trywait(timerSem))
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
            sem_wait(timerSem);
        }
    }
    for(i=0; i<numSensors; i++)
    {
        pthread_join(threadArray[i], NULL);
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
    sem_destroy(timer);
    sem_destroy(sem);
    free(timerSem);
    free(sem);
    return 0;
}