/* Compare folder1 with folder2 and if there is no same ones in
folder2, then make it (include subfolders)*/

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

#define maxsize 65535

int CountProcesses;
int MaxProcesses;
pid_t pid;
int StartLooking(char *, char *);
void WriteFile(char *, char *, struct stat);


int main(int argc, char *argv[])
{
	CountProcesses = 0;
	MaxProcesses = atoi(argv[3]);


	StartLooking(argv[1], argv[2]);

	//Finish last of them
	int status;
	while (CountProcesses > 0) {
		printf("Кол-во процессов - %d\n", CountProcesses);
		wait(&status);
		CountProcesses--;
	}
}


int StartLooking(char *folder1, char *folder2) {
	DIR *cur;
	struct dirent *el;

	cur = opendir(folder1);
	//Open current folder
	if (!(cur))
	{
		fprintf(stderr, "%s: %s\n", folder1, strerror(errno));
		return 1;
	}
	//Read all element in current directory
	while (NULL != (el = readdir(cur))) {
		//Check if our elememt is not our or parent folder1
		if (0 != strcmp(".", el->d_name) && 0 != strcmp("..", el->d_name)) {

			char *path1, *path2;
			path1 = (char*)malloc(PATH_MAX);
			path2 = (char*)malloc(PATH_MAX);
			strcat(strcat(strcpy(path1, folder1), "/"), el->d_name);
			strcat(strcat(strcpy(path2, folder2), "/"), el->d_name);

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
				else
					closedir(dir2);

				StartLooking(path1, path2);

			}
			else if (DT_REG == el->d_type) {
				int status;
				//If we've exceed max count of processes
				//We should wait them to be finished 

				//Check The File not exists
				if (-1 == access(path2, F_OK)) {

					while (CountProcesses >= MaxProcesses) {
						wait(&status);
						CountProcesses--;
					}

					//Make a child process
					CountProcesses++;
					printf("Count of processes is %d\n", CountProcesses);
					pid = fork();

					//If it's a child process then we start work
					if (0 == pid) {

						//Permissions
						struct stat st;
						stat(path1, &st);

						WriteFile(path1, path2, st);
						exit(0);
					}
				}
			}
			free(path1);
			free(path2);
		}
	}
	closedir(cur);
}

void WriteFile(char *p1, char *p2, struct stat st)
{
	//printf("pid process %d\n", getpid());
	//printf("Full path - %s\n", p1);

	ssize_t nread, fullsize;
	char buf[maxsize];
	int fd_from, fd_to;

	fd_from = open(p1, O_RDONLY);
	fd_to = open(p2, O_WRONLY | O_CREAT, st.st_mode);

	fullsize = 0;
	while ((nread = read(fd_from, buf, sizeof(buf))) > 0) {
		char *out = buf;
		ssize_t nwrite;
		do {
			fullsize += nread;

			nwrite = write(fd_to, out, nread);
			if (nwrite >= 0) {
				nread -= nwrite;
				out += nwrite;
			}
		} while (nread > 0);
	}
	printf("pid process %d, Full path - %s,  Full size of copy file - %ld\n\n", getpid(), p1, fullsize);
}

