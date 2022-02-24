#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

sem_t *showroom, *print;
int numGuests;

void* Guest(void* param)
{
    int guestNum = (__intptr_t)param+1, wait;
    srand(pthread_self());
    while(1)
    {
        wait = rand() % 6;
        printf("Guest %d will wander around for %d seconds\n", guestNum, wait);
        sleep(wait);
        sem_wait(print);
        if(sem_trywait(showroom) == 0)
        {
            wait = rand() % 6;
            printf("Guest %d has entered the showroom, set BUSY sign, and will stay for %d seconds\n", guestNum, wait);
            sem_post(print);
            sleep(wait);
            sem_wait(print);
            printf("Guest %d has left the showroom and set AVAILABLE sign\n", guestNum);
            sem_post(showroom);
            sem_post(print);
            return (void*)1;
        }
        else
        {
            printf("Guest %d could not go to the showroom as it was in use\n", guestNum);
            sem_post(print);
        }
    }
}

void GetUserInput()
{
    int i, total = 0;
    char input[1024];
    printf("How many guests will be visiting the showroom? Enter an integer: ");
    while(1)
    {
        i = 0;
        scanf("%s", input);
        while(input[i] > 47 && input[i] < 58)
        {
            i++;
        }
        if(input[i] == 0)
        {
            i = 0;
            while(input[i] != 0)
            {
                total = total * 10 + ((int)input[i] - 48);
                i++;
            }
            if(total >= 100)
            {
                printf("%d guests may take a significant amount of time to complete. Are you sure? (y/n): ", numGuests);
                while(input[0] != 'n')
                {
                    scanf("%s", input);
                    if(input[0] == 'y')
                    {
                        numGuests = total;
                        return;
                    }
                    else if(input[0] == 'n')
                    {
                        total = 0;
                        break;
                    }
                    else
                    {
                        printf("(y/n): ");
                    }
                }
            }
            else
            {
                numGuests = total;
                return;
            }
        }
        printf("Please enter a valid integer: ");
    }
}

int main()
{
    GetUserInput();
    int i;
    showroom = sem_open("/temp", O_CREAT | O_EXCL, 0644, 1);
    sem_unlink("/temp");
    print = sem_open("/temp", O_CREAT | O_EXCL, 0644, 1);
    sem_unlink("/temp");
    pthread_t* threadArray = (pthread_t*)malloc(sizeof(pthread_t) * numGuests);
    for(i=0; i<numGuests; i++)
    {
        pthread_create(&threadArray[i], NULL, Guest, (void*)(__intptr_t)i);
    }
    for(i=0; i<numGuests; i++)
    {
        pthread_join(threadArray[i], NULL);
    }
    sem_close(showroom);
    sem_close(print);
    free(threadArray);
    printf("\nAll guests have attended the showroom. Exiting...\n");
    return 0;
}