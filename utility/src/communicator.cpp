#include "../headers/communicator.h"
#include "../headers/helper_utilities.h"

#include <iostream>
#include <set>
#include <string>
#include <string.h>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <openssl/sha.h>

struct ofile{
    char* filename;
    char* open_sha512_digest;
	char* close_sha512_digest;
	bool written;
	bool read;
	bool created;
};

struct ofile_compare {
    bool operator() (ofile* lhs, ofile* rhs) const{
        return !strcmp(lhs->filename, rhs->filename) && !strcmp(lhs->open_sha512_digest, rhs->open_sha512_digest);
    }
};

static std::set<std::string> files_read;
static std::set<std::string> files_written;

//static std::set<ofile*, ofile_compare> files_openned;
static std::unordered_map<std::string, ofile*> filename_to_ofile;

char* get_sha512(std::string filename){

	unsigned char digest[SHA512_DIGEST_LENGTH];

	std::ifstream inFile (filename.c_str());

	if (!inFile.is_open()){
		printf("Failed to open the file\n");
		return NULL;
	}

    SHA512_CTX mdContext;
    SHA512_Init(&mdContext);

	std::string line;
    while ( getline (inFile, line) ){
        SHA512_Update(&mdContext, line.c_str(), line.length());
    }
    SHA512_Final(digest,&mdContext);

	char* index = (char*) malloc(sizeof(char)*(SHA512_DIGEST_LENGTH*2+1));
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
        sprintf(&index[i*2], "%02x", (unsigned int)digest[i]);

	inFile.close();

    return index;
}

int copy_file(std::string orig_filename, std::string new_filename){

    std::ifstream inFile(orig_filename.c_str());
    std::ofstream toFile(new_filename.c_str());

    if (!inFile.is_open() || !toFile.is_open()){
        if(inFile.is_open()){
            inFile.close();
        }
        if (toFile.is_open()){
            toFile.close();
        }
        return -1;
    }

	std::string line;
    while ( getline (inFile,line) ){
      toFile << line << '\n';
    }

    inFile.close();
    toFile.close();
    return 0;
}

char* file_sha512_and_copy(std::string filename){

    char* sha512_file_digest = get_sha512(filename);

    std::string newfile = file_directory + sha512_file_digest + "_" + std::to_string((int)std::time(NULL));
    copy_file( filename, newfile );

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
		char* digest = file_sha512_and_copy(std::string(file_name));
		ofile* ftemp = (ofile*) malloc(sizeof(struct ofile));

		ftemp->filename = file_name;
		ftemp->open_sha512_digest = digest;
		ftemp->close_sha512_digest = NULL;
		ftemp->written = false;
		ftemp->read = false;
		if (did_create){
			ftemp->created = true;
		}else{
			ftemp->created = false;
		}

		filename_to_ofile[std::string(file_name)] = ftemp;
	}
    return 0;
}


int read_from_file(char* program_name, char* file_name){
    // Method that is invoked when the read is perfromed from a particular file
    // That method is needed to find dependencies for the program
    files_read.insert(std::string(file_name));
	filename_to_ofile[std::string(file_name)]->read = true;
    return 0;
}

int write_to_file(char* program_name, char* file_name){
    // Method that is invoked when the write is happened to a particular file
    // That method is needed to find the files produced by particular programs
    // In here I need to store the file into the config and make it indexable for the finding

    files_written.insert(std::string(file_name));
	filename_to_ofile[std::string(file_name)]->written = true;
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
	if(!isDirectory(file_name)){
		char* digest=file_sha512_and_copy(std::string(file_name));
		filename_to_ofile[std::string(file_name)]->close_sha512_digest = digest;
	}

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

/*
	ofile* t_ofile = filename_to_ofile[filename];
	printf("Filename: %s\n Open Digest: %s\n Close Digest: %s\n Writen: %d\n Read: %d\n Created: %d\n", t_ofile->filename, t_ofile->open_sha512_digest, t_ofile->close_sha512_digest, t_ofile->written, t_ofile->read, t_ofile->created);
*/
    newfile += "/" + filename + "_" + std::to_string(time(0));

	std::ofstream recipe_file(newfile.c_str());

    printf("Writing recipe %s\n", newfile.c_str());

	recipe_file << filename << std::endl << program_name << std::endl;

	for (std::set<std::string>::iterator it=read_files.begin(); it!=read_files.end(); ++it){
        const char* temp_filename = ((std::string)*it).c_str();
		ofile* t_ofile = filename_to_ofile[temp_filename];
		recipe_file << t_ofile->filename << " " << t_ofile->open_sha512_digest << " " << t_ofile->close_sha512_digest << std::endl;
    }

	recipe_file.close();
    return 0;
}
