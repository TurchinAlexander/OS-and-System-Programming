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

#define maxbuf 256

int FolderCount;
int FileCount;


int StartLooking(char *folder, char *filename) {
	DIR *cur;
	struct dirent *el;

	cur = opendir(folder);
	//Open current folder
	if (!(cur)) 
	{
		fprintf(stderr, "%s: %s\n", folder, strerror(errno));
		return 1;
	}	
	//Read all element in current directory
	while (NULL != (el = readdir(cur))) {

		//Check if our elememt is not our or parent folder
		if (0 != strcmp(".", el->d_name) && 0 != strcmp("..", el->d_name)) {

			//Now check if file is a folder //S_ISDIR(x)
			if (DT_DIR == el->d_type) {
				FolderCount++;				

				char *name;
				name=(char*)malloc(PATH_MAX);

				strcpy(name,folder);

				StartLooking(strcat(strcat(name,"/"), el->d_name), filename);
				free(name);
			}
			else if (DT_REG == el->d_type){
				FileCount++;
				//printf("else\n");
				//We found our buddy
				if (0 == strcmp(filename, el->d_name)) {
					char *name;
					name=(char*)malloc(NAME_MAX);

					strcpy(name,folder);

					printf("!FOUND_FILE! %s/%s\n", folder, el->d_name);
					Info(strcat(strcat(name,"/"), el->d_name));	
					free(name);
				}
			}
		}
	}
	closedir(cur);
}

int Info (char *nameoffile)
{
		
	struct stat stat1;	
	stat(nameoffile, &stat1);
	
	//date
	struct tm *tm_ptr;
	time_t Mytime;
	char s1[40]={ 0 };
	Mytime = stat1.st_ctime;
	tm_ptr = gmtime(&Mytime);
	strftime(s1, 80,"%d.%m.%Y %H:%M:%S ",tm_ptr);

	printf("\tДата создания: %s \n\tРазмер: %ld байт \n\tПрава доступа: %o\n\tНомер дескриптора: %lu\n",s1,stat1.st_size,stat1.st_mode,stat1.st_ino);
	printf("\n");
}

int main(int argc, char *argv[], char *envp[]) {

	char folder[maxbuf], path[maxbuf];
	printf("\n");

	//Check for 2 input parameters
	if (argc < 3) {
		printf("Not enough actual parameters\n");
		return 0;
	}
	else {
		strcpy(folder, argv[1]);
		DIR *dir;
		if (NULL == (dir = opendir(folder))) {
			printf("This folder doesn't exist\n");
			return 0;
		}
		else {
			printf("folder else\n");
			closedir(dir);
		}
	}

	StartLooking(argv[1], argv[2]);
	printf("We looked %d folder and %d file except root folder\n", FolderCount, FileCount);
	return 0;
}
