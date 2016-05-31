#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "../headers/communicator.h"
#include "../headers/finder.h"

#include <iostream>
#include <fstream>

extern std::set<std::string> files_read;
extern std::set<std::string> files_written;

int find_recipe_by_sha512(char* sha512_digest){

    std::string newfile = recipe_directory + std::string(sha512_digest);

	DIR* d;
  	struct dirent *dir;
  	d = opendir(newfile.c_str());

  	if (d){
    	while ((dir = readdir(d)) != NULL){
			if (strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")){
                std::string file_name = newfile + "/" + std::string(dir->d_name);

                std::ifstream t_file (file_name.c_str());

				if (!t_file.is_open()){
					continue;
				}

                std::cout << "===============" << std::endl;
                std::string line;

                while (getline(t_file, line)){
                    std::cout << line << std::endl;
                }
                std::cout << std::endl;
			}
		}
		closedir(d);
		return 0;
    }else{
        std::cout << "Recipe doesnot exist!" << std::endl;
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
