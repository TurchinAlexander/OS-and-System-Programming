/*
Write a program to synchronize two directories, for example, Dir1 and Dir2. 
The user specifies the names Dir1 and Dir2. As a result of the program, the files 
that are available in Dir1 but not in Dir2 must be copied to Dir2 together with the access rights. 
Copy procedures should run in a separate thread for each file being copied. 
Each thread displays its id, the full path to the file being copied, and the number of bytes copied. 
The number of concurrent streams N (input by user). 
Copy multiple files from the /etc directory to your home directory. 
Check the program for the /etc directory and the home directory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#define maxsize 65535
#define max 100

typedef struct {
	char path1[PATH_MAX], path2[PATH_MAX];
	struct stat st;
} thread_args;

typedef struct {
	pthread_t tid;
	thread_args arg;
	int isWorking;
} thread_array;

/* Global definitions */
int MaxThreads;
int CountThreads = 0;
int cur_thread = 0;
thread_array arr_thread[max];

/* Prototipes of functions */
int StartLooking(char *, char *);
void *WriteFile(void *);
void Preparations();
void ClearThread(thread_array *);

int main(int argc, char *argv[])
{
	if (argc < 4) {
		printf("Not enough parameters\n");
		return -1;
	}
	MaxThreads = atoi(argv[3]);

	Preparations();
	StartLooking(argv[1], argv[2]);

	//Finish last of them
	printf("Finish\n");
	if (CountThreads > 0) {
		//Go through all array to finish last of them
		for(int i = 0; i < MaxThreads; i++)
		if (1 == arr_thread[i].isWorking) {
			pthread_join(arr_thread[i].tid, NULL);
		}
	}
	printf("Copy has been done successful\n");
}

int StartLooking(char *folder1, char *folder2) {
	DIR *cur;
	struct dirent *el;

	//Open current folder
	cur = opendir(folder1);
	if (!(cur))
	{
		fprintf(stderr, "%s: %s\n", folder1, strerror(errno));
		return 1;
	}
	//Read all element in the current directory
	while (NULL != (el = readdir(cur))) {
		//Check if our elememt is not our or parent folder1
		if (0 != strcmp(".", el->d_name) && 0 != strcmp("..", el->d_name)) {

			char *path1, *path2;
			path1=(char*)malloc(PATH_MAX);
			path2=(char*)malloc(PATH_MAX);
			strcat(strcat(strcpy(path1,folder1),"/"), el->d_name);
			strcat(strcat(strcpy(path2,folder2),"/"), el->d_name);

			//Now check if file is a folder //S_ISDIR(x)
			if (DT_DIR == el->d_type) {
				DIR *dir2;

				//Check the folder in folder2 exists
				if (NULL == (dir2 = opendir(path2))) {
					//Permissions
					struct stat st;
					stat(path1, &st);

					mkdir(path2, st.st_mode);
				}
				else {
					closedir(dir2);
				}
				StartLooking(path1, path2);
			}
			//If it's a file
			else if (DT_REG == el->d_type) {
				//If we've exceed max count of threads
				//We should wait them to be finished
				while (CountThreads >= MaxThreads) {}

				//Check if the File doesn't exists
				if (-1 == access(path2, F_OK)) {
					//Permissions
					struct stat st;
					stat(path1, &st);

					/* Looking for empty space */
					int i = 0;
					while (1 == arr_thread[i].isWorking && i < MaxThreads) {
						i++;
					}

					/* Making arguments for a thread */
					thread_array *temp = &arr_thread[i];
					temp->isWorking = 1;
					strcpy(temp->arg.path1,path1);
					strcpy(temp->arg.path2,path2);
					temp->arg.st = st;

					//Make a new thread
					pthread_create(&arr_thread[i].tid, NULL, WriteFile, (void *)&temp->arg);
					CountThreads++;
				}
			}
			free(path1);
			free(path2);
		}
	}
	closedir(cur);
}

void *WriteFile(void *arg)
{
	long int tid = pthread_self();
	printf("\n\nTid is %ld\n", tid);

	thread_args *temp = (thread_args *)arg;

	printf("Fullpath1 - %s\n", temp->path1);
	printf("Fullpath2 - %s\n", temp->path2);
	printf("Access - %d\n", (*(thread_args *)arg).st.st_mode);

	ssize_t nread, fullsize;
	char buf[maxsize];
	int fd_from, fd_to;

	fd_from = open(temp->path1, O_RDONLY);
	fd_to = open(temp->path2, O_WRONLY | O_CREAT, temp->st.st_mode);

	fullsize = 0;
	while ((nread = read(fd_from, buf, sizeof(buf))) > 0) {
		char *out = buf;
		ssize_t nwrite;
		printf("Read out %ld bytes\n", nread);
		do {
			fullsize += nread;

			nwrite = write(fd_to, out, nread);
			printf("Copied %ld bytes\n", nwrite);
			if (nwrite >= 0) {
				nread -= nwrite;
				printf("Left %ld bytes\n", nread);
				out += nwrite;
			}
		} while (nread > 0);
	}
	close(fd_from);
	close(fd_to);
	printf("Full size of the file - %ld\n\n", fullsize);

	/* Announce that the thread is done */
	int i = 0;
	while (i < MaxThreads) {
		if (tid == arr_thread[i].tid) {
			ClearThread(&arr_thread[i]);
			break;
		}
		i++;
	}
	/* Our thread is done.*/
	CountThreads--;
}

void ClearThread(thread_array *tmp)
{
	tmp->tid = 0;
	tmp->isWorking = 0;
	tmp->arg.path1[0] = '\0';
	tmp->arg.path2[0] = '\0';
}

void Preparations()
{
	for(int i = 0; i < MaxThreads; i++) {
		arr_thread[i].tid = 0;
		arr_thread[i].isWorking = 0;
		arr_thread[i].arg.path1[0] = '\0';
		arr_thread[i].arg.path2[0] = '\0';
		arr_thread[i].isWorking = 0;
	}
}
