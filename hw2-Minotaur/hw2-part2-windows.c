#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

HANDLE showroom, print;
int numGuests;

DWORD Guest(void* param)
{
    int guestNum = (int)param+1, wait;
    srand(GetCurrentThreadId());
    while(TRUE)
    {
        wait = rand() % 6 * 1000;
        printf("Guest %d will wander around for %d seconds\n", guestNum, wait/1000);
        Sleep(wait);
        WaitForSingleObject(print, INFINITE);
        if(WaitForSingleObject(showroom, 0) == WAIT_OBJECT_0)
        {
            wait = rand() % 6 * 1000;
            printf("Guest %d has entered the showroom, set BUSY sign, and will stay for %d seconds\n", guestNum, wait/1000);
            ReleaseSemaphore(print, 1, NULL);
            Sleep(wait);
            WaitForSingleObject(print, INFINITE);
            printf("Guest %d has left the showroom and set AVAILABLE sign\n", guestNum);
            ReleaseSemaphore(showroom, 1, NULL);
            ReleaseSemaphore(print, 1, NULL);
            return 1;
        }
        else
        {
            printf("Guest %d could not go to the showroom as it was in use\n", guestNum);
            ReleaseSemaphore(print, 1, NULL);
        }
    }
    return 1;
}

void GetUserInput()
{
    int i, total = 0;
    char input[1024];
    printf("How many guests will be visiting the showroom? Enter an integer: ");
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
    showroom = CreateSemaphoreA(NULL, 1, 1, NULL);
    print = CreateSemaphoreA(NULL, 1, 1, NULL);
    HANDLE* threadArray = (HANDLE*)malloc(sizeof(HANDLE) * numGuests);
    for(i=0; i<numGuests; i++)
    {
        threadArray[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Guest, (int*)i, 0, NULL);
    }
    for(i=0; i<numGuests; i++)
    {
        WaitForSingleObject(threadArray[i], INFINITE);
        CloseHandle(threadArray[i]);
    }
    CloseHandle(showroom);
    CloseHandle(print);
    free(threadArray);
    printf("\nAll guests have attended the showroom. Exiting...\n");
    return 0;
}