/*
Write a program to synchronize two directories, for example, Dir1 and Dir2. 
The user specifies the names Dir1 and Dir2. 
As a result of the program, the files that are available in Dir1 but not in Dir2 must be copied to Dir2 together with the access rights. 
Copy procedures must be run in a separate process for each file being copied. 
The parent process creates unnamed channels to communicate with the child processes and passes the path, 
	file name, to the channel, reads from the channel, and outputs the results of the copy to the console.  
Child processes write their pid, the full path to the file being copied, and the number of bytes copied to the channel. 
The number of concurrent processes should not exceed N (user input). 
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

#define maxsize 65535
#define FREE -2
#define CANAL_COUNT 100
#define WRITE_CANAL 1
#define READ_CANAL 0

struct {
	pid_t pid;
	int fifo[2];
} canal[CANAL_COUNT];

int p_canal = 0;
int CountProcesses;
int MaxProcesses;
pid_t pid;

int StartLooking(char *, char *);
void WriteFile(int *);
void Preparations();
void Write_In_Canal(char *, char *, struct stat *, int);
void Read_From_Canal(char *, char *, struct stat *, int);
void EmptyCanal(int);


int main(int argc, char *argv[])
{
	CountProcesses = 0;
	MaxProcesses = atoi(argv[3]);
	
	Preparations();
	StartLooking(argv[1], argv[2]);
	
	//Finish last of them
	for (int i = 0; i < MaxProcesses; i++) {
		if (FREE != canal[i].pid) {
			waitpid(canal[i].pid, NULL, 0);
			CountProcesses--;
			
			EmptyCanal(i);
		}
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
			else if (DT_REG == el->d_type){
				//Check The File not exists
				if (-1 == access(path2, F_OK)) {
					//If we've exceed max count of processes
					//We should wait them to be finished 
					while (CountProcesses >= MaxProcesses) {
						waitpid(canal[p_canal].pid, NULL, 0);
						CountProcesses--;
						
						EmptyCanal(p_canal);					
						p_canal = (p_canal + 1) % MaxProcesses;
					}
					
					int i;
					//Find empty place
					for(i = 0; i < MaxProcesses; i++) {
						if(FREE == canal[i].pid) {
							break;
						}
					}

					//Permissions
					struct stat st;
					stat(path1, &st);
					
					//Make a child process
					CountProcesses++;
					pipe(canal[i].fifo);
					Write_In_Canal(path1, path2, &st, canal[i].fifo[WRITE_CANAL]);
					canal[i].pid = fork();
				
					//If it's a child process then we start work
					if (0 == canal[i].pid) {
						//close(canal[i].fifo[WRITE_CANAL]);
						WriteFile(canal[i].fifo);
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

void WriteFile(int *pipe) 
{
	ssize_t nread, fullsize, len;
	char buf[maxsize], p1[PATH_MAX], p2[PATH_MAX];
	int fd_from, fd_to;
	struct stat st;
	
	Read_From_Canal(p1, p2, &st, pipe[READ_CANAL]);

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
	
	/* Send info back to the father */
	len = strlen(p1);
	pid_t pid = getpid();
	write(pipe[WRITE_CANAL], &pid, sizeof(pid_t));
	write(pipe[WRITE_CANAL], &len, sizeof(int));
	write(pipe[WRITE_CANAL], p1, len);
	write(pipe[WRITE_CANAL], &fullsize, sizeof(int));
}

void Preparations(){
	for(int i = 0; i < MaxProcesses; i++) {
		canal[i].pid = FREE;
	}
}

/* Father send info to a child*/
void Write_In_Canal(char *p1, char *p2, struct stat *st, int to)
{
	int len1 = strlen(p1), len2 = strlen(p2);
	
	write(to, &len1, sizeof(int));
	write(to, p1, strlen(p1));
	
	write(to, &len2, sizeof(int));
	write(to, p2, strlen(p2));
	
	write(to, &st->st_mode, sizeof(st->st_mode));
}

/* A child take info from pipe */
void Read_From_Canal(char *p1, char *p2, struct stat *st, int from) 
{
	int len1, len2;
	
	read(from, &len1, sizeof(int)); 
	read(from, p1, len1);

	read(from, &len2, sizeof(int)); 
	read(from, p2, len2);
	
	read(from, &st->st_mode, sizeof(st->st_mode));

}

/*Clean place for second usage*/
void EmptyCanal(int ind) 
{
	int fd_from = canal[ind].fifo[READ_CANAL];
	int len, fullsize;
	pid_t pid;
	char p[PATH_MAX];
	
	read(fd_from, &pid, sizeof(pid_t));
	read(fd_from, &len, sizeof(int));
	read(fd_from, p, len);
	read(fd_from, &fullsize, sizeof(int));
	
	printf("pid process %d \nFull path - %s \nFull size of copy file - %d\n\n", pid, p, fullsize);
	
	canal[ind].pid = FREE;
	close(canal[ind].fifo[WRITE_CANAL]);
	close(canal[ind].fifo[READ_CANAL]);
}

