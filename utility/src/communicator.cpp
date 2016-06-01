#include "../headers/communicator.h"
#include "../headers/helper_utilities.h"

#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

#include <openssl/sha.h>

static std::set<std::string*, set_string_compare> files_read;
static std::set<std::string*, set_string_compare> files_written;

//static std::set<ofile*, ofile_compare> files_openned;
static std::unordered_map<std::string, ofile*> filename_to_ofile;
static char* parent_cwd;

char* get_sha512(std::string filename){

	unsigned char digest[SHA512_DIGEST_LENGTH];

	std::ifstream inFile (filename.c_str());

	if (!inFile.is_open()){
		return NULL;
	}

    SHA512_CTX mdContext;
    SHA512_Init(&mdContext);

	std::string line;
    while ( getline (inFile, line) ){
        SHA512_Update(&mdContext, line.c_str(), line.length());
    }
    SHA512_Final(digest,&mdContext);

	char* index = new char[SHA512_DIGEST_LENGTH*2+1];
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

std::string* file_sha512_and_copy(std::string filename){

    char* sha512_file_digest = get_sha512(filename);
    if (!sha512_file_digest){
        return NULL;
    }

    std::string timer = std::to_string(std::time(NULL));
    std::string newfile = file_directory + sha512_file_digest + "_" + timer;
    copy_file( filename, newfile );

    return new std::string(sha512_file_digest);
}

void initiate_communication(char* cwd){
    struct stat st;
    parent_cwd = cwd;

    if (stat(file_directory.c_str(), &st) == -1) {
        mkdir(file_directory.c_str(), 0700);
    }
    if (stat(recipe_directory.c_str(), &st) == -1) {
        mkdir(recipe_directory.c_str(), 0700);
    }
}

int opened_file(std::string file_name, bool did_create){
/*    if(file_name[0] == '/'){
        std::cout << "WARNING: ABSOLUTE PATH FOR THE FILE" << std::endl;
    }*/

    // Method invoked when a file is openned.
    // Need to check if the file is not some sysfile and make a copy of the file here
    // Save it to the control file as well
	if(!isDirectory(file_name)){
        std::string* digest = file_sha512_and_copy(std::string(file_name));
		ofile* ftemp = new ofile;

		ftemp->filename = file_name;
		ftemp->open_sha512_digest = *digest;
		ftemp->written = false;
		ftemp->read = false;
        ftemp->created = did_create;
        ftemp->closed = false;

		filename_to_ofile[std::string(file_name)] = ftemp;
	}
    return 0;
}


int read_from_file(std::string file_name){
    // Method that is invoked when the read is perfromed from a particular file
    // That method is needed to find dependencies for the program
    files_read.insert(new std::string(file_name));
	filename_to_ofile[std::string(file_name)]->read = true;
    return 0;
}

int write_to_file(std::string file_name){
    // Method that is invoked when the write is happened to a particular file
    // That method is needed to find the files produced by particular programs
    // In here I need to store the file into the config and make it indexable for the finding

    files_written.insert(new std::string(file_name));
	filename_to_ofile[std::string(file_name)]->written = true;
    return 0;
}

int rename_file(std::string from, std::string to){
    filename_to_ofile[std::string(to)] = filename_to_ofile[std::string(from)];

    std::string* to_find = new std::string(from);
    if(files_read.find(to_find) != files_read.end()){
        files_read.insert(new std::string(to));
    }
    if(files_written.find(to_find) != files_written.end()){
        files_written.insert(new std::string(to));
    }

    delete to_find;
	return 0;
}

int file_close(std::string file_name){
    // Note if the file was closed without any changes (why was it open in the first place?)
	//TODO: need to be carefyul here. It might be the case that the file is moved, without a closed handle
	//TODO: add the outlined check
	if(!isDirectory(file_name)){
        std::string* digest=file_sha512_and_copy(file_name);
        if (!digest){
            return -1;
        }
		filename_to_ofile[file_name]->close_sha512_digest = *digest;
		filename_to_ofile[file_name]->closed = true;
	}

    return 0;
}

int should_track(std::string file_name){
    //TODO: maybe add a set of rules here or something?
    if ( file_name[0] != '/' || (file_name[0] == '/' && file_name.find(getlogin()) != std::string::npos)){
        return 1;
    }
    return 0;
}

int close_communication(std::string program_name){
    // Need to process the file here to create the dependencies file

    for(std::unordered_map<std::string, ofile*>::iterator it=filename_to_ofile.begin(); it!=filename_to_ofile.end(); ++it){
        if (!it->second->closed){
           file_close(it->second->filename.c_str());
           it->second->closed=true;
        }
    }

    for(std::set<std::string*>::iterator it=files_written.begin(); it!=files_written.end(); ++it){
        std::string* index = file_sha512_and_copy(*(std::string*)*it);
        if (!index){
            continue;
        }
        write_recipe(*(std::string*)*it, *index, program_name);
    }
    return 0;
}

int write_recipe(std::string filename, std::string sha512_digest, std::string program_name){

    std::string newfile = recipe_directory + sha512_digest;

    struct stat st;
    if (stat(newfile.c_str(), &st) == -1) {
        mkdir(newfile.c_str(), 0700);
    }

    //TODO: yeah, was too smart. I see why they preserve the hierarchy
    newfile += "/_" + std::to_string(time(0));
    /*if(filename[0] == '/'){
        newfile += "/_" + std::to_string(time(0));
    }else{
        newfile += "/" + filename + "_" + std::to_string(time(0));
    }
    */
	std::ofstream recipe_file(newfile.c_str());

	recipe_file << filename << std::endl << parent_cwd << std::endl << program_name << std::endl;

	for (std::set<std::string*>::iterator it=files_read.begin(); it!=files_read.end(); ++it){
        const char* temp_filename = (*(std::string*)*it).c_str();
		ofile* t_ofile = filename_to_ofile[temp_filename];
		recipe_file << t_ofile->filename << " " << t_ofile->open_sha512_digest << " " << t_ofile->close_sha512_digest << std::endl;
    }

	recipe_file.close();
    return 0;
}
