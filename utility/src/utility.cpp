#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../headers/tracer.h"
#include "../headers/communicator.h"
#include "../headers/finder.h"

#include <iostream>

void traceme(int argc, char* argv[]){

    char cwd[100];
    getcwd(cwd, sizeof(cwd));

    initiate_communication(cwd);

    int pn_size = 0;
    for(int i=1; i<argc; i++){
        pn_size += strlen(argv[i]) + 1;
    }
    pn_size+=1;

    char* pn = new char[pn_size];

    strcpy(pn, argv[1]);
    for(int i=2; i < argc; i++){
        strcat(pn, " ");
        strcat(pn, argv[i]);
    }

	trace(argc, argv, pn);

    close_communication(std::string(pn));

    delete[] pn;
}

int main (int argc, char *argv[]){
    if (argc < 2){
        printf("Usage: %s -f filename_to_find | -m sha512_of_file | command_to_execute\n", argv[0]);
        return 1;
    }

	int r, opt;
	while ((opt = getopt (1, argv, "f:m:")) != -1){
		switch (opt){
		case 'f':
			printf ("Finding the file: %s\n", optarg);
			r = find_recipe_by_name(std::string(optarg));
			if (r < 0){
                std::cout << "File not found" << std::endl;
            }
            return r;
		case 'm':
		    printf ("Looking for sha512 of the file %s\n", optarg);
            r = find_recipe_by_sha512(std::string(optarg));
            if (r < 0){
                std::cout << "File not found" << std::endl;
            }
		    return r;
	    }
    }
	traceme(argc, argv);
    return 0;
}
