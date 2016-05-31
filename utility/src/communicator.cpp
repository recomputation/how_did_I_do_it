#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/sha.h>

#include "../headers/communicator.h"
#include "../headers/helper_utilities.h"

#include <iostream>
#include <set>
#include <string>

static std::set<std::string> files_read;
static std::set<std::string> files_written;


char* get_sha512(std::string filename){

	unsigned char digest[SHA512_DIGEST_LENGTH];

	FILE* inFile = fopen (filename.c_str(), "rb");

	//TODO: check if the file is accessible
	if (!inFile){
		printf("Failed to open the file\n");
		return NULL;
	}

    SHA512_CTX mdContext;
    int bytes;
	int dataSize = 1024;
    char data[dataSize];

    SHA512_Init(&mdContext);
    while ((bytes = fread (data, 1, dataSize, inFile)) != 0){
        SHA512_Update(&mdContext, data, bytes);
	}
    SHA512_Final(digest,&mdContext);

	char* index = (char*) malloc(sizeof(char)*(SHA512_DIGEST_LENGTH*2+1));
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
        sprintf(&index[i*2], "%02x", (unsigned int)digest[i]);

    fclose(inFile);

    return index;
}

int copy_file(std::string orig_filename, std::string new_filename){
    FILE* toFile = fopen(new_filename.c_str(), "wb+");
	FILE* inFile = fopen (orig_filename.c_str(), "rb");

    if(!toFile || !inFile){
        return -1;
    }

	int dataSize = 1024;
    char data[dataSize];
    ssize_t nread;
	while ((nread = fread(data, 1, dataSize, inFile)) > 0){
        char* out_ptr = data;
        ssize_t nwritten;
        do {
            nwritten = fwrite(out_ptr, 1, nread, toFile);
            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
        } while (nread > 0);
    }

	fclose(inFile);
	fclose(toFile);
    return 0;
}

char* file_sha512_and_copy(std::string filename){

    char* sha512_file_digest = get_sha512(filename);

    std::string newfile = file_directory + sha512_file_digest;
    copy_file( filename, newfile);

    return sha512_file_digest;
}

void initiate_communication(){
    struct stat st;

    if (stat(file_directory.c_str(), &st) == -1) {
        mkdir(file_directory.c_str(), 0700);
    }
    if (stat(recipe_directory.c_str(), &st) == -1) {
        mkdir(recipe_directory.c_str(), 0700);
    }
}

int opened_file(char* program_name, char* file_name, int did_create){
    // Method invoked when a file is openned.
    // Need to check if the file is not some sysfile and make a copy of the file here
    // Save it to the control file as well
    printf("Open %s by %s(%d)\n", file_name, program_name, did_create);
	if(!isDirectory(file_name)){
		free(file_sha512_and_copy(std::string(file_name)));
	}
    return 0;
}

int read_from_file(char* program_name, char* file_name){
    // Method that is invoked when the read is perfromed from a particular file
    // That method is needed to find dependencies for the program
    files_read.insert(std::string(file_name));
    return 0;
}

int write_to_file(char* program_name, char* file_name){
    // Method that is invoked when the write is happened to a particular file
    // That method is needed to find the files produced by particular programs
    // In here I need to store the file into the config and make it indexable for the finding

    files_written.insert(std::string(file_name));
    return 0;
}

int rename_file(char* program_name, char* from, char* to){
	return -1;
}

int file_close(char* program_name, char* file_name){
    // Note if the file was closed without any changes (why was it open in the first place?)
    printf("Close %s by %s\n", file_name, program_name);
	//TODO: need to be carefyul here. It might be the case that the file is moved, without a closed handle
	//TODO: add the outlined check
	if(!isDirectory(file_name))
		free(file_sha512_and_copy(std::string(file_name)));
    return 0;
}

int should_track(char* file_name){
    //TODO: maybe add a set of rules here or something?
    if ( file_name[0] != '/' || (file_name[0] == '/' && strstr(file_name, getlogin()))){
        return 1;
    }
    return 0;
}

int close_communication(char* program_name){
    // Need to process the file here to create the dependencies file

    for(std::set<std::string>::iterator it=files_written.begin(); it!=files_written.end(); ++it){
        char* index = file_sha512_and_copy((std::string)*it);
        //TODO: do something with the arguments and the file itself
        write_recipe((std::string)*it, index, program_name, files_read);
        free(index);
    }
    return 0;
}

int write_recipe(std::string filename, char* sha512_digest, char* program_name, std::set<std::string> read_files){

    std::string newfile = recipe_directory + std::string(sha512_digest);

    struct stat st;
    if (stat(newfile.c_str(), &st) == -1) {
        mkdir(newfile.c_str(), 0700);
    }

    newfile += "/" + filename;
    FILE* recipe_file = fopen (newfile.c_str(), "wb+");

    printf("Writing recipe %s\n", newfile.c_str());

    fwrite(filename.c_str(), 1, strlen(filename.c_str()), recipe_file);
    fwrite("\n", 1, 1, recipe_file);
	fwrite("COMMAND:", 1, 8, recipe_file);
    fwrite(program_name, 1, strlen(program_name), recipe_file);
    fwrite("\n", 1, 1, recipe_file);
	fwrite("DEPEND:", 1, 7, recipe_file);
	for (std::set<std::string>::iterator it=read_files.begin(); it!=read_files.end(); ++it){
        const char* temp = ((std::string)*it).c_str();
        fwrite(temp, 1, strlen(temp), recipe_file);
        fwrite(" ", 1, 1, recipe_file);
    }

    fclose(recipe_file);

    return 0;
}
