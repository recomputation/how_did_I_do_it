#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "../headers/communicator.h"

extern std::set<std::string> files_read;
extern std::set<std::string> files_written;

int find_recipe_by_md5(char* md5_digest){

    std::string newfile = recipe_directory + std::string(md5_digest);

	DIR* d;
  	struct dirent *dir;
  	d = opendir(newfile.c_str());
	char buffer[100];

  	if (d){
    	while ((dir = readdir(d)) != NULL){
			if (strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")){
                std::string file_name = newfile + "/" + std::string(dir->d_name);

				FILE* t_file = fopen( file_name.c_str(), "r");

				if (!t_file){
					continue;
				}

				printf("===============\n");

				while (fgets(buffer, sizeof(buffer), t_file)) {
					printf("%s", buffer);
				}
				fclose(t_file);

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
