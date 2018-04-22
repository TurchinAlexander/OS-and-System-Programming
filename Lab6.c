/* 
Write a program to copy large ( > 2GB ) files by multiple processes at the same time. 
The user specifies the N-number of concurrent processes and the names of the input and output files. 
To copy, the input file is split into N parts and each part is copied in a separate process. 
To solve the problem of mutual exclusion while simultaneously writing to the output file, use a semaphore. 
The number of processes at any given time must be equal to N. 
*/

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <limits.h>
#include <semaphore.h>


void CopyFile(char *, char *, int);
void CopyPart(char *, int , size_t , off_t , size_t );
void Write_in_file(int , char *, size_t , off_t);

sem_t sem_name;

int main(int argc, char **argv)
{
	/* Checking for errors */
	if (argc < 4) {
		printf("count of parameters is wrong\n");
		return 1;
	}
	
	int fd;
	if(-1 == (fd = open(argv[1], O_RDONLY))) {
		printf("Cannot find copy file\n");
		return 1;
	}
	else {
		close(fd);
	}
	
	int process_count = atoi(argv[3]);
	if(process_count <= 0) {
		printf("Wrong count of processes\n");
		return 1;
	}
	
	/* Start working */
	
	CopyFile(argv[1], argv[2], process_count);
	return 0;
}

void CopyFile(char *source, char *dest, int process_count)
{
	struct stat statbuf;
	pid_t pid;
	
	stat(source, &statbuf);
	
	int dest_fd;
	dest_fd = open(dest, O_CREAT | O_WRONLY, statbuf.st_mode);
	
	size_t part_size = (size_t)statbuf.st_size / process_count;
	off_t part_offset = 0;
	
	sem_init(&sem_name, 1, 1);
	
	for(int i = 0; i < process_count; i++) {
		pid = fork();
		if (0 == pid) {
			CopyPart(source, dest_fd, statbuf.st_blksize, part_offset, i != process_count ? part_size : (statbuf.st_size - part_offset));
			exit(0);
		}
		else {
			part_offset += part_size;
		}
	}
	
	/* Finish work */
	while (-1 != wait(NULL)) {}
	
	close(dest_fd);
	
	printf("Job is done\n");
}

void CopyPart(char *source, int dest_fd, size_t bufsize, off_t part_offset, size_t part_size) 
{
    off_t part_end = part_offset + part_size;
    char buf[bufsize];

    int source_fd;
    source_fd = open(source, O_RDONLY);

    lseek(source_fd, part_offset, SEEK_SET);

    while (part_offset < part_end) {
        size_t bytes_count;
        int is_last = (part_end - part_offset <= bufsize) ? 1 : 0;
        
        bytes_count = read(source_fd, buf, 1 == is_last ? (size_t) (part_end - part_offset) : bufsize);
        
        Write_in_file(dest_fd, buf, (size_t) bytes_count, part_offset);
        
        part_offset += bytes_count;
        
    }


    close(source_fd);
}

void Write_in_file(int dest_fd, char *outbuf, size_t outbuf_size, off_t part_offset) 
{
	sem_wait(&sem_name);
	
	lseek(dest_fd, part_offset, SEEK_SET);
	
	write(dest_fd, outbuf, outbuf_size);
	
	sem_post(&sem_name);
}



