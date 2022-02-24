#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

sem_t** semArray;
int cupcake = 1, alert = 0, numGuests;

void* Leader(void* param)
{
    int guestNum = (__intptr_t)param, count = 0;
    srand(pthread_self());
    while(1)
    {
        sem_wait(semArray[guestNum]);
        if(cupcake)
        {
            printf("Guest %d found a cupcake and ate it\n", guestNum);
            cupcake = 0;
            if(++count == numGuests)
            {
                alert = 1;
                printf("Guest %d will alert the minotaur that all guests have entered the labyrinth at least once\n", guestNum);
                sem_post(semArray[0]);
                return (void*)1;
            }
        }
        else
        {
            printf("Guest %d did not find a cupcake", guestNum);
            if(rand() % 2)
            {
                printf(", requested another, and ate it\n");
            }
            else
            {
                printf("and chose not to request another\n");
            }
        }
        sem_post(semArray[0]);
    }
}

void* Guest(void* param)
{
    int guestNum = (__intptr_t)param, firstEmpty = 1;
    srand(pthread_self());
    while(1)
    {
        sem_wait(semArray[guestNum]);
        if(alert)
        {
            return (void*)1;
        }
        if(cupcake)
        {
            printf("Guest %d found a cupcake and did not eat it\n", guestNum);
        }
        else
        {
            printf("Guest %d did not find a cupcake", guestNum);
            if(firstEmpty)
            {
                
                printf(", requested another, and did not eat it\n");
                firstEmpty = 0;
                cupcake = 1;
            }
            else
            {
                if(rand() % 2)
                {
                    printf(", requested another, and ate it\n");
                }
                else
                {
                    printf("and chose not to request another\n");
                }
            }
        }
        sem_post(semArray[0]);
    }
}

void* Minotaur(void* param)
{
    int num;
    srand(pthread_self());
    while(1)
    {
        num = rand() % numGuests + 1;
        printf("Minotaur has chosen guest %d to enter the labyrinth\n", num);
        sem_post(semArray[num]);
        sem_wait(semArray[0]);
        if(alert)
        {
            printf("The minotaur is pleased!\n");
            for(num=0; num<numGuests; num++)
            {
                sem_post(semArray[num+1]);
            }
            return (void*)1;
        }
    }
}

void GetUserInput()
{
    int i, total = 0;
    char input[1024];
    printf("How many guests will be particpating in the game? Enter an integer: ");
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
            if(total >= 10)
            {
                printf("The will create %d threads. Are you sure? (y/n): ", numGuests);
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
            else if(total == 0 || total == 1)
            {
                total = 0;
                printf("There must be at least two guests to play the game\n");
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
    srand(pthread_self());
    int i, leaderNum = rand()%numGuests+1;
    semArray = (sem_t**)malloc(sizeof(sem_t*) * (numGuests+1));
    pthread_t* threadArray = (pthread_t*)malloc(sizeof(pthread_t) * (numGuests+1));
    printf("\n%d guests are attending the party\nGuest %d was appointed the leader during the guests's planning period\n\n", numGuests, leaderNum);
    for(i=1; i<numGuests+1; i++)
    {
        semArray[i] = sem_open("/temp", O_CREAT | O_EXCL, 0644, 0);
        sem_unlink("/temp");
        if(i != leaderNum)
        {
            pthread_create(&threadArray[i], NULL, Guest, (void*)(__intptr_t)i);
        }
        else
        {
            pthread_create(&threadArray[i], NULL, Leader, (void*)(__intptr_t)i);
        }
    }
    semArray[0] = sem_open("/temp", O_CREAT | O_EXCL, 0644, 0);
    sem_unlink("/temp");
    pthread_create(&threadArray[0], NULL, Minotaur, NULL);
    for(i=0; i<numGuests+1; i++)
    {
        pthread_join(threadArray[i], NULL);
        sem_close(semArray[i]);
    }
    free(threadArray);
    free(semArray);
    printf("\nThe game has officially ended. Exiting...\n");
    return 1;
}