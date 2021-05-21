#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

bool INDEXED = false;
bool LONGLIST = false;
bool RECURSED = false;

void getDirListRecursive(struct dirent **list, int size, char *path);
void printDirList(struct dirent **list, int size, char *path);
void printFilePermission(struct stat input);

int main(int argc, char const *argv[]){
	char cwd[1000];
	struct dirent **files;
	int size = 0;
	if(argc == 1){
		getcwd(cwd, sizeof(cwd));
		size = scandir(cwd, &files, NULL, alphasort);
		printDirList(files, size, cwd);
	}else if(argc == 2){
		char *argument = malloc(50);
		strcpy(argument, argv[1]);
		if(argument[0] == '-'){
			getcwd(cwd, sizeof(cwd));
			size = scandir(cwd, &files, NULL, alphasort);
			if(strstr(argv[1], "R") != NULL){
				RECURSED = true;
			}
			if(strstr(argv[1], "l") != NULL){
				LONGLIST = true;
			}
			if(strstr(argv[1], "i") != NULL){
				INDEXED = true;
			}
			printDirList(files, size, cwd);
		}else{
			size = scandir(argument, &files, NULL, alphasort);
			printDirList(files, size, argument);
		}
	}else if(argc == 3){
		char *filename = argv[2];
		if(strstr(argv[1], "R") != NULL){
			getDirListRecursive(files, size, filename);
		}
		if(strstr(argv[1], "l") != NULL){
			LONGLIST = true;
		}
		if(strstr(argv[1], "i") != NULL){
			INDEXED = true;
		}
		size = scandir(filename, &files, NULL, alphasort);
		printDirList(files, size, filename);

	}else{
		printf("%s\n", "Too many arguments.");
	}
	return 0;
}

void getDirListRecursive(struct dirent **list, int size, char *path){
	char curPath [1000];
	DIR *dir = opendir(path);
	struct dirent tempList[50];

	if(dir == NULL){
		fprintf(stderr, "can't open %s\n",path);
		return;
	}

	for(int i = 0; i < size; i++){
			if (strcmp(list[i] -> d_name, ".") != 0 && strcmp(list[i]-> d_name, "..") != 0){
				// sortutil(list, size);
				printf("%s\n", list[i]->d_name);
				if((list[i] -> d_type) == DT_DIR){
					struct dirent **files;
					int numDir_ent = 0;

					strcpy(curPath, path);
					strcat(curPath,"/");
					strcat(curPath, list[i] -> d_name);
					numDir_ent = scandir(curPath, &files, NULL, alphasort);

					getDirListRecursive(files, numDir_ent, curPath);	
				}				
			}
	}

	closedir(dir);
}

int sortList( const void *const A, const void *const B){

	return strcmp((*(struct dirent **)A)->d_name,(*(struct dirent **)B)->d_name);
}

void printDirList(struct dirent **list, int size, char *path){
	
		struct stat temp;
		if(INDEXED == true && LONGLIST == false && RECURSED == false){
			//Implemented "ls -i"
			for(int i = 0; i < size; i++){
				if (strcmp(list[i] -> d_name, ".") != 0 && strcmp(list[i]-> d_name, "..") != 0){
					stat(list[i]->d_name, &temp);
					printf(" %lu ", temp.st_ino);
					printf("%s\n", list[i]->d_name);
				}else printf("%s\n", list[i]->d_name);
			}
		}else if(INDEXED == false && LONGLIST == true && RECURSED == false){
			//-l
			for(int i = 0; i < size; i++){
				if (strcmp(list[i] -> d_name, ".") != 0 && strcmp(list[i]-> d_name, "..") != 0){
					stat(list[i]->d_name, &temp);
					printFilePermission(temp);
					printf("  %lu ", temp.st_nlink);
					struct passwd *pw = getpwuid(temp.st_uid);
					struct group *gr = getgrgid(temp.st_gid);
					printf("%s ", pw->pw_name);
					printf("%s ", gr->gr_name);
					printf("%-6lu ", temp.st_size);
					char date[20];
					strftime(date, 20, "%b %d %G %R", localtime(&(temp.st_ctime)));
					printf("%s ", date);
					printf("%s\n", list[i]->d_name);
				}
			}
		}else if(RECURSED == true && LONGLIST == false && INDEXED == false){
			//Called "ls -R"
			getDirListRecursive(list, size, path);

		}else if(INDEXED == true && LONGLIST == true && RECURSED == false){
			// Implemented "ls -il"
			for(int i = 0; i < size; i++){
				stat(list[i]->d_name, &temp);
				printf(" %lu ", temp.st_ino);
				printFilePermission(temp);
				printf(" %lu ", temp.st_nlink);
				struct passwd *pw = getpwuid(temp.st_uid);
				struct group *gr = getgrgid(temp.st_gid);
				printf("%s ", pw->pw_name);
				printf("%s ", gr->gr_name);
				printf("%-6lu ", temp.st_size);
				char date[20];
				strftime(date, 20, "%b %d %G %R", localtime(&(temp.st_ctime)));
				printf("%s ", date);
				printf("%s\n", list[i]->d_name);
			}
		}else if(INDEXED == true && LONGLIST == false && RECURSED == true){
			//Implemented "ls -iR"
			for(int i = 0; i < size; i++){
				if (strcmp(list[i] -> d_name, ".") != 0 && strcmp(list[i]-> d_name, "..") != 0){
					stat(list[i]->d_name, &temp);
					printf(" %lu ", temp.st_ino);
					printf("%s\n", list[i]->d_name);
				}
			}
			printf("\n");
			for(int i = 0; i < size; i++){
				char curPath [1000];
				struct dirent **tempList;
				int numDir_ent = 0;

				if (strcmp(list[i] -> d_name, ".") != 0 && strcmp(list[i]-> d_name, "..") != 0 && (list[i] -> d_type) == DT_DIR){
					
					printf("./%s :\n", list[i] -> d_name);	

					if((list[i] -> d_type) == DT_DIR){

						strcpy(curPath, path);
						strcat(curPath,"/");
						strcat(curPath, list[i] -> d_name);
						numDir_ent = scandir(curPath, &tempList, NULL, alphasort);
						DIR *dir = opendir(curPath);
						//getDirListRecursive(tempList, numDir_ent, curPath);

						for(int i = 0; i < numDir_ent; i++){
							if (strcmp(tempList[i] -> d_name, ".") != 0 && strcmp(tempList[i]-> d_name, "..") != 0){
								stat(tempList[i]->d_name, &temp);
								printf(" %lu ", temp.st_ino);
								printf("%s\n", tempList[i]->d_name);
							}
						}

					}

				}
						
			}
		
		}else if(INDEXED == false && LONGLIST == true && RECURSED == true){
			//Implemented "ls -lR"
			for(int i = 0; i < size; i++){
			
				if (strcmp(list[i] -> d_name, ".") != 0 && strcmp(list[i]-> d_name, "..") != 0){
						stat(list[i]->d_name, &temp);
						printFilePermission(temp);
						printf("  %lu ", temp.st_nlink);
						struct passwd *pw = getpwuid(temp.st_uid);
						struct group *gr = getgrgid(temp.st_gid);
						printf("%s ", pw->pw_name);
						printf("%s ", gr->gr_name);
						printf("%-6lu ", temp.st_size);
						char date[20];
						strftime(date, 20, "%b %d %G %R", localtime(&(temp.st_ctime)));
						printf("%s ", date);
						printf("%s\n", list[i]->d_name);
					}
			}
			printf("\n");

			for(int i = 0; i < size; i++){

				char curPath [1000];
				struct dirent **tempList;

				if (strcmp(list[i] -> d_name, ".") != 0 && strcmp(list[i]-> d_name, "..") != 0 && (list[i] -> d_type) == DT_DIR){
					
					printf("./%s :\n", list[i] -> d_name);

					if((list[i] -> d_type) == DT_DIR){
						struct dirent **files;
						int numDir_ent = 0;

						strcpy(curPath, path);
						strcat(curPath,"/");
						strcat(curPath, list[i] -> d_name);
						numDir_ent = scandir(curPath, &tempList, NULL, alphasort);
						DIR *dir = opendir(curPath);
						//getDirListRecursive(tempList, numDir_ent, curPath);
						for(int i = 0; i < numDir_ent; i++){
			
							if (strcmp(tempList[i] -> d_name, ".") != 0 && strcmp(tempList[i]-> d_name, "..") != 0){
									stat(tempList[i]->d_name, &temp);
									printFilePermission(temp);
									printf("  %lu ", temp.st_nlink);
									struct passwd *pw = getpwuid(temp.st_uid);
									struct group *gr = getgrgid(temp.st_gid);
									printf("%s ", pw->pw_name);
									printf("%s ", gr->gr_name);
									printf("%-6lu ", temp.st_size);
									char date[20];
									strftime(date, 20, "%b %d %G %R", localtime(&(temp.st_ctime)));
									printf("%s ", date);
									printf("%s\n", tempList[i]->d_name);
								}
						}
					}
				}
			}
		}else if(INDEXED == true && LONGLIST == true && RECURSED == true){
			//Implemented "ls -iRl"
			for(int i = 0; i < size; i++){
				stat(list[i]->d_name, &temp);
				printf(" %lu ", temp.st_ino);
				printFilePermission(temp);
				printf(" %lu ", temp.st_nlink);
				struct passwd *pw = getpwuid(temp.st_uid);
				struct group *gr = getgrgid(temp.st_gid);
				printf("%s ", pw->pw_name);
				printf("%s ", gr->gr_name);
				printf("%-6lu ", temp.st_size);
				char date[20];
				strftime(date, 20, "%b %d %G %R", localtime(&(temp.st_ctime)));
				printf("%s ", date);
				printf("%s\n", list[i]->d_name);
			}
			for(int i = 0; i < size; i++){

				char curPath [1000];
				struct dirent **tempList;

				if (strcmp(list[i] -> d_name, ".") != 0 && strcmp(list[i]-> d_name, "..") != 0 && (list[i] -> d_type) == DT_DIR){
					
					printf("./%s :\n", list[i] -> d_name);

					if((list[i] -> d_type) == DT_DIR){
						struct dirent **files;
						int numDir_ent = 0;

						strcpy(curPath, path);
						strcat(curPath,"/");
						strcat(curPath, list[i] -> d_name);
						numDir_ent = scandir(curPath, &tempList, NULL, alphasort);
						DIR *dir = opendir(curPath);
						getDirListRecursive(tempList, numDir_ent, curPath);
						printf("%s\n", tempList[i]->d_name);
					}
				}
			}

		}else{
			for(int i = 0; i < size; i++){
				printf(" ");
				printf("%s\n", list[i]->d_name);
			}
		}	
}
void printFilePermission(struct stat input){
	if(input.st_mode & S_IRUSR){
		printf("r");
	}else{
		printf("-");
	}
	if(input.st_mode & S_IWUSR){
		printf("w");
	}else{
		printf("-");
	}
	if(input.st_mode & S_IXUSR){
		printf("x");
	}else{
		printf("-");
	}
	if(input.st_mode & S_IRGRP){
		printf("r");
	}else{
		printf("-");
	}
	if(input.st_mode & S_IWGRP){
		printf("w");
	}else{
		printf("-");
	}
	if(input.st_mode & S_IXGRP){
		printf("x");
	}else{
		printf("-");
	}
	if(input.st_mode & S_IROTH){
		printf("r");
	}else{
		printf("-");
	}
	if(input.st_mode & S_IWOTH){
		printf("w");
	}else{
		printf("-");
	}
	if(input.st_mode & S_IXOTH){
		printf("x");
	}else{
		printf("-");
	}
	printf("  ");
}