#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "../headers/communicator.h"
#include "../headers/finder.h"

extern std::set<std::string> files_read;
extern std::set<std::string> files_written;

int find_recipe_by_sha512(char* sha512_digest){

    std::string newfile = recipe_directory + std::string(sha512_digest);

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
	char* sha512_of_file = get_sha512(filename);

	if(!sha512_of_file){
		printf("File access issues");
		return -1;
	}

	return find_recipe_by_sha512(sha512_of_file);
}
