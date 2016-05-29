#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "../headers/communicator.h"

extern const char* file_directory;
extern const char* recipe_directory;

int find_recipe_by_md5(char* md5_digest){

    char* newfile = malloc(sizeof(char)*(1+strlen(md5_digest)+strlen(recipe_directory)));
    strcpy(newfile, recipe_directory);
    strcat(newfile, md5_digest);

	DIR* d;
  	struct dirent *dir;
  	d = opendir(newfile);
	char buffer[100];

  	if (d){
    	while ((dir = readdir(d)) != NULL){
			if (strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")){
				int stringsize = 2+strlen(newfile) + strlen(dir->d_name);
				char* file_name = malloc(sizeof(char)*(stringsize));
				strcpy(file_name, newfile);
				strcat(file_name, "/");
				strcat(file_name, dir->d_name);
				file_name[stringsize]='\0';

				FILE* t_file = fopen( file_name, "r");

				if (!t_file){
					free(file_name);
					continue;
				}

				printf("===============\n");

				while (fgets(buffer, sizeof(buffer), t_file)) {
					printf("%s", buffer);
				}
				fclose(t_file);
				free(file_name);
				printf("\n");
			}
		}
		closedir(d);
		return 0;
    }else{
        printf("Recipe doesnot exist!\n");
        return -1;
    }
}

int find_recipe_by_name(char* filename){
	char* md5_of_file = get_md5(filename);

	if(!md5_of_file){
		printf("File access issues");
		return -1;
	}

	return find_recipe_by_md5(md5_of_file);
}
