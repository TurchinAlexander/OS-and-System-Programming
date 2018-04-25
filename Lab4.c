/*
The processes continuously exchange signals according to the `table 2`.
A record in `table 1` of the form 1->(2,3,4,5) means that source process 0 creates child process 1, which in turn creates child processes 2,3,4,5.
The entry in `table 2` of the following type: 1 - >(2,3,4) SIGUSR1 means that process 1 sends a SIGUSR1 signal to child processes 2,3,4 simultaneously (i.e. for one call to kill ()).
Each process displays the following information on the console when it receives or sends a signal:
N pid ppid sent / received USR1 / USR2 current time (µsec)
where N is the number of son at the table.
One Process 1, after receiving the 101'st SIGUSR signal, sends the SIGTERM signal to the sons and waits for all the sons to complete, and then completes itself.
Process 0 waits for process 1 to complete and then completes itself. Sons, having received the SIGTERM signal, shut down the console output of the view message:
pid ppid completed work after the X-th SIGUSR1 signal and Y-signal SIGUSR2
where X – Y is the number of SIGUSR1 and SIGUSR2 signals sent by the son during the whole operation

To create the correct sequence of signals in accordance with the task table, it is necessary for each process to write its own signal processor in
which it (the process) receives a signal from the previous (in the table) process and sends the next (in the table) process!!

All jobs must have error control (if any directory is not accessible, you must display the appropriate message and continue execution).
The output of error messages should be produced in the standard output stream for error messages (stderr) in the following form:
module_name: the text of the message.
Example: a pid : 1.exe : open file Error: 1.txt
Module name, file name are taken from the command line arguments.

`table 1` (tree of processes) : 1->(2,3,4,5) 5->(6,7)   7->8
`table 2` (where to send signal) : 1->2 SIGUSR1   2->(3,4) SIGUSR2   4->5 SIGUSR1   3->6 SIGUSR1  6->7 SIGUSR1  7->8 SIGUSR2   8->1 SIGUSR2

*/

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>  
#include <unistd.h> 
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

#define MYSIGSTOP 25
#define MAXSIGCOUNT 10

static int i = 1;
static int isfinished = 0;
int arrpid[] = {0,0,0,0,0,0,0,0,0,0}; 

void send (int dest, int sign)
{
	if (isfinished <= MAXSIGCOUNT)
	{
		if (dest == 3 || dest == 4) {
			printf("%d process, %d - pid, %d - ppid, send signal =%d= to group %d, time %ld\n", i, getpid(), getppid(), sign, arrpid[dest], clock());
			killpg(arrpid[dest], sign);
		}
		else {
			printf("%d process, %d - pid, %d - ppid, send signal =%d= to %d, time %ld\n", i, getpid(), getppid(), sign, dest, clock());
			kill (arrpid[dest], sign);
		}
	}
}
void showmess (int sig)
{
	isfinished++;
	usleep(1000);
	printf("-->>> %d process %d - pid, %d - ppid, get signal =%d=, time %ld\n", i, getpid(), getppid(), sig, clock());
}
void my_handler1 (int sig)
{
	printf("\n\nnext cicle\n");
	usleep(100000);
	showmess(sig);	
	send(2, SIGUSR1);		
}
void my_handler2 (int sig)
{
	showmess(sig);	
	if (sig == MYSIGSTOP) {isfinished = MYSIGSTOP; exit;}
	send(3, SIGUSR2);
}
void my_handler3 (int sig)
{
	showmess(sig);	
	if (sig == MYSIGSTOP) {isfinished = MYSIGSTOP; exit;}
	send(6, SIGUSR1);
}
void my_handler4 (int sig)
{
	showmess(sig);	
	if (sig == MYSIGSTOP) {isfinished = MYSIGSTOP; exit;}
	send(5, SIGUSR1);
}
void my_handler5 (int sig)
{
	showmess(sig);	
	if (sig == MYSIGSTOP) {isfinished = MYSIGSTOP; exit;}
}
void my_handler6 (int sig)
{
	showmess(sig);	
	if (sig == MYSIGSTOP) {isfinished = MYSIGSTOP; exit;}
	send(7, SIGUSR1);
}
void my_handler7 (int sig)
{
	showmess(sig);	
	if (sig == MYSIGSTOP) {isfinished = MYSIGSTOP; exit;}
	send(8, SIGUSR1);
}
void my_handler8 (int sig)
{
	showmess(sig);	
	if (sig == MYSIGSTOP) {isfinished = MYSIGSTOP; exit;}
	send(1, SIGUSR2);
}


int main () 
{     
	pid_t pid;
	pid = fork();
	if (pid == 0) {
		pid = 1;
		arrpid[1] = getpid();
		//Make a tree of processes
		for (int j = 2; j <= 5; j++)
		{
			if (pid > 0)
			{
				pid = fork();
				if (pid == 0)
				{
					i = j;

				}
				else if (pid > 0)
				{
					arrpid[j] = pid;
				}
			}
		}
		FILE *filepid;
		if (i == 5)
		{
			arrpid[5] = getpid();
			pid = fork();
			if (pid == 0) // 6 
			{
				i = 6;
			}
			else if (pid > 0) // 7
			{
				arrpid[6] = pid;
				pid = fork();
				if (pid == 0)
				{
					i = 7;
					arrpid[i] = getpid();
					pid = fork();
					if (pid == 0) // 8
					{
						i = 8;
						arrpid[i] = getpid();
						filepid = fopen("allpids.txt", "wb+");
						fwrite(arrpid, sizeof(pid_t), 9, filepid);
						fclose(filepid);
					}
				}
			}
		}
		if (i < 8)
		{
			usleep(10000);
		}
		else
		{
			printf("\n");
		}
	

		filepid = fopen ("allpids.txt","rb+");
		fread(&arrpid, sizeof(pid_t), 9, filepid);
		fclose(filepid);
	
		//Set same group
		arrpid[4] = arrpid[3];
	
		printf("pid of %d is %d\n", i, getpid());
		setpgid(getpid(), arrpid[i]);
	
		usleep(100000);

		if (i == 1) {signal (SIGUSR1, my_handler1);signal (SIGUSR2, my_handler1);}
		if (i == 2) {signal (SIGUSR1, my_handler2);signal (SIGUSR2, my_handler2); signal (MYSIGSTOP, my_handler8);}
		if (i == 3) {signal (SIGUSR1, my_handler3);signal (SIGUSR2, my_handler3); signal (MYSIGSTOP, my_handler8);}
		if (i == 4) {signal (SIGUSR1, my_handler4);signal (SIGUSR2, my_handler4); signal (MYSIGSTOP, my_handler8);}
		if (i == 5) {signal (SIGUSR1, my_handler5);signal (SIGUSR2, my_handler5); signal (MYSIGSTOP, my_handler8);}
		if (i == 6) {signal (SIGUSR1, my_handler6);signal (SIGUSR2, my_handler6); signal (MYSIGSTOP, my_handler8);}
		if (i == 7) {signal (SIGUSR1, my_handler7);signal (SIGUSR2, my_handler7); signal (MYSIGSTOP, my_handler8);}
		if (i == 8) {signal (SIGUSR1, my_handler8);signal (SIGUSR2, my_handler8); signal (MYSIGSTOP, my_handler8);}	

		if (i == 1) 
		{	
			printf("/// ");
			for (int h = 1;h<=8; h++)
				printf("%d ", arrpid[h]);
			printf("///\n"); 
			//isfinished--;
			printf("cool sinal\n");
			usleep(9000);
			printf("Send signal to 2 %d\n", getpid());
			send (2, SIGUSR1);
		}

		
		while (1)
		{
			if (isfinished >= MAXSIGCOUNT)
			{
				break;
			}
		}

	
		if (i == 1)
		{
			printf("The main proc is waiting...\n");		
			for (int j = 8; j>=2; j--)
			{	
				send(j, MYSIGSTOP); 
				usleep(1000);
				kill(arrpid[j], SIGKILL);
			}
			usleep(1000);
			exit;
		}
		else
		{
			raise(SIGSTOP);
		}
	}
	else {	
		printf("Waiting %d\n", getpid());
		wait(NULL);
	}
	return (0);
}
