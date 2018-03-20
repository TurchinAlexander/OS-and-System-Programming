/*Написать программу синхронизации двух каталогов, например, Dir1 и Dir2.
Пользователь задаёт имена Dir1 и Dir2.В результате работы программы файлы, имеющиеся в Dir1, 
но отсутствующие в Dir2, должны скопироваться в Dir2 вместе с правами доступа.
Процедуры копирования должны запускаться в отдельном процессе для каждого копируемого файла.
Каждый процесс выводит на экран свой pid, полный путь к копируемому файлу и число скопированных байт.
Число одновременно работающих процессов не должно превышать N(вводится пользователем).
Скопировать несколько файлов из каталога / etc в свой домашний каталог.
Проверить работу программы для каталога / etc и домашнего каталога.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#define maxsize 256

int CountProcesses;
int MaxProcesses;
int StartLooking(char *, char *);
void WriteFile(char *, char *, struct stat );


int main(int argc, char *argv[])
{
	CountProcesses = 0;
	MaxProcesses = *argv[3];
	StartLooking(argv[1], argv[2]);
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
				//char *path1, *path2;
				//path1=(char*)malloc(PATH_MAX);
				//path2=(char*)malloc(PATH_MAX);
				
				//Into the depths 
				//strcpy(path1,folder1);
				//strcpy(path2,folder2);
				//strcat(strcat(strcpy(path1,folder1),"/"), el->d_name);
				//strcat(strcat(strcpy(path2,folder2),"/"), el->d_name);
				
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
				
				//free(path1);
				//free(path2);
			}
			else if (DT_REG == el->d_type){
				//printf("else\n");
				//Check The File not exists
				if (access(path2, F_OK) != -1) {
					//Permissions
					struct stat st;
					stat(path1, &st);
					
					WriteFile(path1, path2, st);
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
	ssize_t nread;
	char buf[maxsize];
	int fd_from, fd_to;
	
	fd_from = open(p1, O_RDONLY);
	fd_to = open(p2, O_WRONLY | O_CREAT, st.st_mode);
	
	while ((nread = read(fd_from, buf, sizeof(buf))) > 0) {
		char *out;
		ssize_t nwrite;
		
		do {
			nwrite = write(fd_to, out, nread);
			
			if (nwrite >= 0) {
				nread -= nwrite;
				out += nwrite;
			}
		} while (nread > 0);
	}		
}

