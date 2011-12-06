/*
 * ReadMe.txt
 * Ryan Konkul
 * rkonku2
 * CS 385
 * Homework 4: vmpager
 */
 
Optional enhancements:
I implement an algorithm termed least_used where the victim frame picked to be paged out is a page that is least used. To enable it, there is a const bool at the top of the program which when set to true, turns on the least_used algorithm. Otherwise it uses the first in first out algorithm. 

Description of program:
This program simulates a simple paging system. The first command argument is the input file. Other optional command arguments are #processes MB_RAM maxReads fractionWrite. 
 
The program begins by parsing the command arguments and opening a file specified in the command arguments. If a file is found, I calculate the file size. If it is greater than 5000, then fseek is used to seek 5000 bytes into the file. If less than 5000, it will not fseek. Once a file is opened, processes and their memory accesses are read in. Until the file ends a process is composed of pid, num_reads, and mem_accesses. A process is only saved if pid and num_reads is read. If the file ends before mem_accesses is done, the process is still saved.
 
A frame table composed of a pid and page_number (mem_access) is initialized with max_size of MB_RAM * 4. If MB_RAM is zero, the maximum frame table size is then 256 * MB_RAM. Then all processes do their mem_accesses. If the page is in the frame table, the clock is incremented by 1 and then the next memory access is done. If it is not in the frame table, a frame may need to be kicked out. The frame table is a linked list so the front() is removed and the new page is pushed to the back. If the page was dirty, then 40000 is added to the clock to write out the page to disk. Then with at least one frame free, the new page is loaded and 40001 is added to the clock. 

Analysis of Algorithms:
Using diver.bmp and parameters num_processes=64, infinite memory, maxReads=512, and fractionWrite = 2, the statistics given are
2855 frametablesize
2855 page misses
12244 page hits
114215099 time units

This is the case where no page was paged out due to the frame table being full. 
Using the least_used algorithm, and mb_ram to zero, the performance was the same as first in first out

Changing the MB_RAM to 10, First in first out algorithm is used and the statistics were
40 frametablesize
5678 page misses
9421 page hits
292455099 time units

With MB_RAM set to 10 and least_used algorithm
40 frametablesize
12747 page misses
2352 page hits
652665099 time units

The least_used algorithm performed significantly worse than first in first out in almost all cases. It may be because of some bug in the algorithm. It may also be because if all accesses are used the same amount, the algorithm degenerates to first in last out.

diver.bmp has a somewhat low temporal locality so halving the size of the frame table did not hurt a great amount. This is because many pages are loaded only once and never used again. Compared to the infinite memory, the time units increased by 109.8%. 

Now, using diver.bmp and parameters num_processes=64, infinite memory, maxReads=512, and fractionWrite = 2
95 frametablesize
95 page misses
139 page hits
3800234 time units

As before changing only MB_RAM to 10
40 frametablesize
152 page misses
82 page hits
8160234 time units

Time units increased by 114.7%. Running the diver.bmp and Chevalier_473.bmp on the same parameters gave a very similar increase in time units. 

Infinite memory in every case performed better than first in first out. 





 