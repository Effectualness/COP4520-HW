Summary:
The following programs to synchronize the two homework problems was written in C, with two seperate implementations for linux/macOS and windows. Due to the nature of the grading, the repository was made private again, with the proof given in an image in this folder (proof.png on 4/7/22) against the date of this folders first commit (4/12/22). Each program will start immediately, but if a user may change certain variables. At the beginning of each program, there are a set of values that determine the number of threads and such. They are indicated with a comment above them. Note that the print variable may be set to 1, which will print live information of tasks being completed for the user, to confirm that the program is working as intended.

Part 1 begins by creating an array of random unique values. Thenm threads are created for all servants. Each servant randomly chooses to perform one of three tasks. Task 1 involves adding a present to the linked lists. This is accomplished by using hand-over-hand locking on each nodes mutex, to ensure that there are no conflicts from the multiple threads that may be acting on it. The second task involves removing the head of the list. There exists a special semaphore to ensure that the head is available from other threads that are either adding of removing from the list.  That last task involves checking for a tag in the list. This is generated randomly, and has threads iterate through the list in the same manner as the add method, but without making any changes. After all the unsorted presents are added/removed, the servants return data on what tasks they accomplished, which is printed for the user. If the print flag is set, each servant will print whenever they add to the list (and where), remove, or whether or not they could find a tag in the list.

Part 2 treats the main function as the CPU, and each sensor as a thread. To syncronize the two, a timer is used, and may be adjusted by the user. The concept of the timer works as such: Whenver a set amount of time has passed, the timer is signaled (or a seperate semaphore is signaled for linux), and the sensors perform their readings. The main CPU must accomplish its task before this timer is complete. The way this is checked is to first check if it is signaled. If it is, the main CPU missed its time, and will print an error message and the program will end. If the timer is not signaled, the main CPU is on time and will now wait until the timer is signaled. This is the guarantee that the tasks are being performed on time. There are some additional syncronization methods in place for the CPU to wait for the sensors to write their new data. The main CPU will manipulate this data while the timer is running, so there is no post data processing after an hour has completed. Once all hours are completed, all the requested data is printed for the user, include the highest temperatures, lowest temperatures, and the largest difference along with the period it first occurred. If the print flag is set, all the above value is printed for each loop, along with the random values generated. Note that the differences are only printed for each period, which occurs every 10 loops, starting from 9. As for ensuring these programs work, the original specification was for the readings to occur every minute. On windows, the program has shown to work consistently every microsecond, and for linux, every 10 milliseconds. With this in mind, it will certainly work if the readings occur every minute.

To Compile (from hw3-Minotaur-p2 directory) on windows:
gcc hw3-part1-windows.c -o hw3-part1-windows.exe
OR
gcc hw3-part2-windows.c -o hw3-part2-windows.exe

To Compile (from hw3-Minotaur-p2 directory) on linux/macOS:
gcc -pthread hw3-part1-linux.c -lrt
OR
gcc -pthread hw3-part2-linux.c -lrt



To Execute (from hw3-Minotaur-p2 directory) on windows:
hw3-part1-windows.exe
OR
hw3-part2-windows.exe

To Execute (from hw3-Minotaur-p2 directory) on linux/macOS:
./a.out
OR
./a.out