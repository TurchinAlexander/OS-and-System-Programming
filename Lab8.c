#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <math.h>

#define MODE 0777
#define WRITE_CANAL 1
#define READ_CANAL 0

void StartWork();
void Process1(int);
void Process2(int, int);
void Process3(int);


char FileOutput[PATH_MAX];
int N, n;

int main (int argc, char **argv) 
{
	strcpy(FileOutput, "Output.txt");
	
	if (argc < 3) {
		fprintf(stderr, "Two few argument");
		return 1;
	} 
	
	if (N = atoi(argv[1]), N == 0) {
		fprintf(stderr, "Error or too small number");
		return 1;
	}
	
	if (n = atoi(argv[2]), n == 0) {
		fprintf(stderr, "Error or too small number");
		return 1;
	}
	
	StartWork();
	return 0;
}


void StartWork() 
{
	pid_t pid[3];
	
	int pipe_gr[2], pipe_out[2];
	
	pipe(pipe_gr);
	pipe(pipe_out);
	
	if (pid[0] = fork(), pid[0] == 0) {
		Process1(pipe_gr[WRITE_CANAL]);
		exit(0);
	} 
	
	if (pid[1] = fork(), pid[1] == 0) {
		Process2(pipe_gr[READ_CANAL], pipe_out[WRITE_CANAL]);
		exit(0);
	}
	
	if (pid[2] = fork(), pid[2] == 0) {
		Process3(pipe_out[READ_CANAL]);
		exit(0);
	}

	for(int i = 1; i < 3; i++) {
		waitpid(pid[i], NULL, 0);
	}
	
}

void Process1(int pipe1_write) 
{ 	
	for(int i = 0; i < N; i++) {
		double ch, x;
		
		x = (2 * M_PI * i) / N;
		ch = x;	
		
		write(pipe1_write, (void *)&ch, sizeof(ch));
		
		for(int j = 1; j < n; j++) {
			ch = -(ch*x*x)/((2*j)*(2*j+1)); // x * (x^2/(n(n+1)))
			write(pipe1_write, (void *)&ch, sizeof(ch));
		}
	}
}

void Process2(int pipe1_read, int pipe2_write) 
{
	double sum, curmemb;
	for(int i = 0; i < N; i++) {
		sum = 0;
		
		for(int j = 0; j < n; j++) {
			read(pipe1_read, &curmemb, sizeof(curmemb));
			sum += curmemb;
		}
		write(pipe2_write, (void *)&sum, sizeof(sum));
	}
}

void Process3(int pipe2_read) 
{
	FILE *f_out;
	double membsum;
	char *msg = "Result of sin";	
	
	f_out = fopen(FileOutput, "w+");
	
	for(int i = 0; i < N; i++) {
		read(pipe2_read, &membsum, sizeof(membsum));
		fprintf(f_out, "%s[%d] - %f\n", msg, i, membsum);
	}
	fclose(f_out);
}
