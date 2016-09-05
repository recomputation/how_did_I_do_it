#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "../headers/tracer.h"
#include "../headers/communicator.h"
#include "../headers/finder.h"
#include "../headers/helper_utilities.h"

#include <iostream>

void traceme(int argc, char* argv[], bool verbose){

    char cwd[100];
    getcwd(cwd, sizeof(cwd));

    initiate_communication(cwd);

	trace(argc, argv, cwd, verbose);

    std::string t_pn = "";

    for (int i=0; i < argc; i++){
        t_pn += std::string(argv[i]);
    }

    close_communication(t_pn);
}

int main (int argc, char *argv[]){
    if (argc < 2){
        std::cout << "Usage: " << argv[0] << " -f filename_to_find | -m sha512 sha512_of_file | -b buildenv | command_to_execute" << std::endl;
        return 1;
    }

    bool verbose = false, findme = false, findmysha = false, builder = false;
    std::string findme_a, findmysha_a, builder_a;

    for(int i=1; i < argc; i++){
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
            continue;
        }

        if (strcmp(argv[i], "-f") == 0){
            findme = true;
            findme_a = argv[i+1];
            break;
        }

        if (strcmp(argv[i], "-b") == 0){
            builder = true;
            builder_a = argv[i+1];
            break;
        }

        if (strcmp(argv[i], "-m") == 0 ){
            findmysha = true;
            findmysha_a = argv[i+1];
            break;
        }

        argc -= i;
        argv += i;
        break;
    }

    int r;

    if (builder){
        std::cout << "Building the environment for the file: " << builder_a << std::endl;
        r = build_environment(builder_a);
        if (r < 0){
            std::cout << "File not found" <<std::endl;
        }
        return r;
    }

    if (findme){
        std::cout << "Attempting to find the file: " << findme_a << std::endl;
        r = find_recipe_by_name(findme_a);
        if (r < 0){
            std::cout << "File not found" << std::endl;
        }
        return r;
    }

    if (findmysha){
        std::cout << "Attempting to find the sha512 of the file: " << findmysha_a << std::endl;
        r = find_recipe_by_sha512(findmysha_a);
        if (r < 0){
            std::cout << "File not found" << std::endl;
        }
        return r;
    }

    load_file_configs();
	traceme(argc, argv, verbose);
    return 0;
}
