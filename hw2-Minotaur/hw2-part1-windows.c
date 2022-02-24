#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

HANDLE* semArray;
int cupcake = 1, alert = 0, numGuests;

DWORD Leader(void* param)
{
    int guestNum = (int)param, count = 0;
    srand(GetCurrentThreadId());
    while(TRUE)
    {
        WaitForSingleObject(semArray[guestNum], INFINITE);
        if(cupcake)
        {
            printf("Guest %d found a cupcake and ate it\n", guestNum);
            cupcake = 0;
            if(++count == numGuests)
            {
                alert = 1;
                printf("Guest %d will alert the minotaur that all guests have entered the labyrinth at least once\n", guestNum);
                ReleaseSemaphore(semArray[0], 1, NULL);
                return 1;
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
        ReleaseSemaphore(semArray[0], 1, NULL);
    }
}

DWORD Guest(void* param)
{
    int guestNum = (int)param, firstEmpty = 1;
    srand(GetCurrentThreadId());
    while(TRUE)
    {
        WaitForSingleObject(semArray[guestNum], INFINITE);
        if(alert)
        {
            return 1;
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
        ReleaseSemaphore(semArray[0], 1, NULL);
    }
}

DWORD Minotaur(void* param)
{
    int num;
    srand(GetCurrentThreadId());
    while(TRUE)
    {
        num = rand() % numGuests + 1;
        printf("Minotaur has chosen guest %d to enter the labyrinth\n", num);
        ReleaseSemaphore(semArray[num], 1, NULL);
        WaitForSingleObject(semArray[0], INFINITE);
        if(alert)
        {
            printf("The minotaur is pleased!\n");
            for(num=0; num<numGuests; num++)
            {
                ReleaseSemaphore(semArray[num+1], 1, NULL);
            }
            return 1;
        }
    }
}

void GetUserInput()
{
    int i, total = 0;
    char input[1024];
    printf("How many guests will be particpating in the game? Enter an integer: ");
    while(TRUE)
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
    srand(GetCurrentThreadId());
    int i, leaderNum = rand()%numGuests+1;
    semArray = (HANDLE*)malloc(sizeof(HANDLE) * (numGuests+1));
    HANDLE* threadArray = (HANDLE*)malloc(sizeof(HANDLE) * (numGuests+1));
    printf("\n%d guests are attending the party\nGuest %d was appointed the leader during the guests's planning period\n\n", numGuests, leaderNum);
    for(i=1; i<numGuests+1; i++)
    {
        semArray[i] = CreateSemaphoreA(NULL, 0, 1, NULL);
        if(i != leaderNum)
        {
            threadArray[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Guest, (int*)i, 0, NULL);
        }
        else
        {
            threadArray[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Leader, (int*)i, 0, NULL);
        }
    }
    semArray[0] = CreateSemaphoreA(NULL, 0, 1, NULL);
    threadArray[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Minotaur, NULL, 0, NULL);
    for(i=0; i<numGuests+1; i++)
    {
        WaitForSingleObject(threadArray[i], INFINITE);
        CloseHandle(threadArray[i]);
        CloseHandle(semArray[i]);
    }
    free(threadArray);
    free(semArray);
    printf("\nThe game has officially ended. Exiting...\n");
    return 1;
}