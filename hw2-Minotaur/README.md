Summary:
The following programs to synchronize the two homework problems was written in C, with two seperate implementations for linux/macOS and windows. Due to the nature of the grading, the repository was made private again, with the proof given in an image in this folder (proof.png on 2/19/22) against the date of this folders first commit (2/22/22). Each program starts with a prompt for the user to enter an integer, with some checks to ensure that the user entered a valid number, and warnings for large numbers. An array of threads is created for each, along with semaphores for each program. Each thread in each program is also seeded use the current threads id to ensure different values. Both programs end with a print statement to indicate the program ran to completion, ensuring threads have completed and been freed.

Part 1 creates threads for all guests (count specified by user, being at least 2) and the minotaur. At the beginning, one guest is randomly chosen, and is created as a leader instead of a guest. Each guest (including the leader) is halted by its own semaphore, which is contained in an array of semaphores, whose count is equal to the number of guests specified by the user, and is at count 0 at the start of the program. The minotaur will choose a random guest, and release that guests semaphore. The minotaur will then wait on its own semaphore, which will be released by the guest at its loop completion. To ensure the win condition is met, guests must follow a specific algorithm, differing from the leader. Whenever a guest reaches the end of the labyrinth, the guest must leave the state of the cupcake as they found it (i.e.: Guest found a cupcake, must leave cupcake). However, the first time they find an empty plate, they must refill it, and leave it. This is the information that signals the leader that another, unique guest has entered the labyrinth. Afterwards, finding an empty plate means they must leave it empty, which they can do by simply leaving or refilling then eating it, which is chosen randomly. The leader also follows this last cause of finding an empty plate and leaving it empty. However, when the leader finds a cupcake, they must eat it to allow guests to refill it. The leader keeps a personal count how of many times they found a cupcake, and when that number reaches the number of guests, they can know for sure that every guest has entered the labyrinth at least once. The leader can then signal the minotaur to end the game, who in turn ends each guest by releasing all semaphores with the alert flag already set.

Part 2 has the minotaur's second option implemented. The second option was chosen as the best of the three options. Option two allows for guests to perform other tasks while they wait for the showroom, which is not possible for option three, and for a much greater amount of time than option one. However, since each thread must periodically check to see if it can go to the showroom, its runs slower than the other options. In terms of implementation, each guest's thread sleeps for a random amount of time between 0 and 5 seconds. They then check for a semaphore that indicates if the showroom is available. If it is, they obtain the semaphore and wait another random amount of time before releasing it. If it is not, the guest will wait for another random amount of time before trying again. Due note that due to the speed of c, print statements can be inaccurate, so another semaphore was implemented to help with the flow of the print statements. When each guest leaves the showroom, they leave, and the program ends when all guests have been to the showroom once.

To Compile (from hw2-Minotaur directory) on windows:
gcc hw2-part1-windows.c -o hw2-part1-windows.exe
OR
gcc hw2-part2-windows.c -o hw2-part2-windows.exe

To Compile (from hw2-Minotaur directory) on linux/macOS:
gcc -pthread hw2-part1-linux.c -o hw2-part1-linux.exe
OR
gcc -pthread hw2-part2-linux.c -o hw2-part2-linux.exe



To Execute (from hw1-Prime-Finder directory) on windows:
./hw2-part1-windows.exe
OR
./hw2-part2-windows.exe

To Execute (from hw1-Prime-Finder directory) on linux/macOS:
./hw2-part1-linux.exe
OR
./hw2-part2-linux.exe