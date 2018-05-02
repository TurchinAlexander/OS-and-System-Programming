#OS and System Programming


Lab2: 
``` 
  ./lab2.c file folder  
``` 
  Find the *file* in the *folder*. The program is also searching in subfolders. If any file is found, then print info about it to the terminal.  


Lab3:
```  
  ./lab3.c folder1 folder2 N  
```  
  Compare *folder1* with *folder2*. If there is no same ones in *folder2*, then make it (include subfolders).  
  Copying is used in N specialized processes, which return pid, filename, total number of bytes scanned, and comparison results.  


Lab4:
```  
  ./lab4.c
```   
  Make a group of processes and show the interaction between them through signals SIGUSR1 and SIGUSR2.  
  Tree of processes: 1->(2,3,4,5) 5->(6,7)   7->8  
  Where to send signal: 1->2 SIGUSR1   2->(3,4) SIGUSR2   4->5 SIGUSR1
     3->6 SIGUSR1 6->7 SIGUSR1  7->8 SIGUSR2   8->1 SIGUSR2


Lab5:
```  
  gcc … -lpthread 
  ./lab5.c folder1 folder2 N
```  
  Just the same, as Lab3, but instead of processes there are threads.


Lab6:
```  
  gcc … -lpthread  
  ./Lab6.c filename1 filename2 N
```   
  The program is copying file *filename1* in *N* processes at the same time in *filename2*. 
  To copy, the input file is split into *N* parts and each part is copied in a separate process. 
  To solve the problem of mutual exclusion while simultaneously writing to the output file, use a semaphore. 
  The number of processes at any given time must be equal to *N*. 


Lab7:
```  
  gcc … -lpthread
  ./Lab7.c folder1 folder2 N
```  
  Just the same, as Lab3, but there you should ‘talk’ with ‘child’ process through pipelines. 
  Each ‘child’ process is given info through a pipeline: file and where to copy and access rights to the file.
  Each ‘child’ should return his pid, filename and full size of the file.


Lab8:
```  
  gcc … -lpthread  
  ./Lab8.c N n
```  
  Write a program to find the array of function values y[i]=sin(2 &times; PI &times; i / *N*) (i = 0,1,2...*N*-1) using Taylor series.  
  The program is split in 3 processes.  
  1 process: counting *n* Taylor series members of sin and transfer them to 2 process.  
  2 process: summation of Taylor series members for a particular sin and transfer result to 3 process.  
  3 process: write results of calculations in a file.