#include "../headers/communicator.h"
#include "../headers/helper_utilities.h"

#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include <algorithm>

#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <openssl/sha.h>

std::set<std::string> files_read;
std::set<std::string> files_written;
std::set<std::string> need_dirs;

std::string file_directory;
std::string recipe_directory;

std::string timer;
std::unordered_map<std::string, openned_file> filename_to_ofile;
char* parent_cwd;

std::string get_rand_char_seq(int length){
    char temp[length];
    srand(time(NULL));

    for (int i = 0; i < length; i++){
        temp[i] = 'A' + (std::rand()%26);
    }
    temp[length]='\0';
    return temp;
}


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
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++){
        sprintf(&index[i*2], "%02x", (unsigned int)digest[i]);
    }
	inFile.close();

    return index;
}


std::string* file_sha512_and_copy(std::string filename){

    char* sha512_file_digest = get_sha512(filename);
    if (!sha512_file_digest){
        return NULL;
    }

    std::string newfile = file_directory + sha512_file_digest + "_" + timer;
    copy_file( filename, newfile );

    std::string* tt = new std::string(sha512_file_digest);
    delete[] sha512_file_digest;
    return tt;
}

void initiate_communication(char* cwd, std::string t_pn){
    struct stat st;
    parent_cwd = cwd;
    timer = std::to_string(std::time(NULL)) + "_" + sha512_string(t_pn) + "_" + get_rand_char_seq(10);
    if (stat(file_directory.c_str(), &st) == -1) {
        mkdir(file_directory.c_str(), 0700);
    }
    if (stat(recipe_directory.c_str(), &st) == -1) {
        mkdir(recipe_directory.c_str(), 0700);
    }
}

int opened_file(std::string file_name, bool did_create){
    // Method invoked when a file is openned.
    // Need to check if the file is not some sysfile and make a copy of the file here
    // Save it to the control file as well
    //

	if(!isDirectory(file_name)){
        std::string* digest = file_sha512_and_copy(std::string(file_name));
        if( !digest ){
            return -1;
        }
		openned_file ftemp;
		ftemp.filename = file_name;
		ftemp.open_sha512_digest = *digest;
		ftemp.close_sha512_digest = "";
        ftemp.created = did_create;
		ftemp.written = false;
		ftemp.read    = false;
        ftemp.closed  = false;

        delete digest;

		filename_to_ofile[file_name] = ftemp;
	}else{
	    if (need_dirs.find(file_name) == need_dirs.end()){
            need_dirs.insert(file_name);
        }
    }
    return 0;
}


int read_from_file(std::string file_name){
    // Method that is invoked when the read is perfromed from a particular file
    // That method is needed to find dependencies for the program

    if (filename_to_ofile.find(file_name) != filename_to_ofile.end()){
	    filename_to_ofile[file_name].read = true;
    }

    if (files_read.find(file_name) == files_read.end()){
        files_read.insert(file_name);
    }
    return 0;
}

int write_to_file(std::string file_name){
    // Method that is invoked when the write is happened to a particular file
    // That method is needed to find the files produced by particular programs
    // In here I need to store the file into the config and make it indexable for the finding
    if (filename_to_ofile.find(file_name) != filename_to_ofile.end()){
	    filename_to_ofile[file_name].written = true;
    }

    if (files_written.find(file_name) == files_written.end()){
        files_written.insert(file_name);
    }

    return 0;
}

int rename_file(std::string from, std::string to){
    // Not sure if that is the best way to handle rename
    if (filename_to_ofile.find(from) != filename_to_ofile.end()){
        filename_to_ofile[to] = filename_to_ofile[from];
    }

    if((files_read.find(from) != files_read.end()) && (files_read.find(to) == files_read.end())){
        files_read.insert(to);
    }

    if((files_written.find(from) != files_written.end()) && (files_written.find(to) == files_written.end())){
        files_written.insert(to);
    }

	return 0;
}

int file_close(std::string file_name){
	if(!isDirectory(file_name) && (filename_to_ofile.find(file_name) != filename_to_ofile.end())){
        std::string* digest=file_sha512_and_copy(file_name);
        if (!digest){
            return -1;
        }

        struct stat st;
        stat(file_name.c_str(), &st);
        filename_to_ofile[file_name].permissions = st.st_mode;
		filename_to_ofile[file_name].close_sha512_digest += *digest;
		filename_to_ofile[file_name].closed = true;
		delete digest;
	}
    return 0;
}

int close_communication(std::string program_name){
    // Need to process the file here to create the dependencies file

    for(std::unordered_map<std::string, openned_file>::iterator it=filename_to_ofile.begin(); it!=filename_to_ofile.end(); ++it){
        if (!it->second.closed){
           file_close(it->second.filename.c_str());
           it->second.closed=true;
        }
    }

    for(std::set<std::string>::iterator it=files_written.begin(); it!=files_written.end(); ++it){
        std::string* index = file_sha512_and_copy((std::string)*it);
        if (!index){
            continue;
        }
        write_recipe(*it, *index, program_name);
        delete index;
    }


    files_read.clear();
    files_written.clear();
    need_dirs.clear();
    filename_to_ofile.clear();

    return 0;
}

int write_recipe(std::string filename, std::string sha512_digest, std::string program_name){

    std::string newfile = recipe_directory + sha512_digest;

    struct stat st;
    if (stat(newfile.c_str(), &st) == -1) {
        mkdir(newfile.c_str(), 0700);
    }

    newfile += "/_" + timer;
    std::cout << "[" << getpid() << "] Writing into " << newfile << " " << filename << " " << program_name << std::endl;

	std::ofstream recipe_file(newfile.c_str());

	recipe_file << filename << std::endl << parent_cwd << std::endl << program_name << std::endl;

	for (std::set<std::string>::iterator it=files_read.begin(); it!=files_read.end(); ++it){
		openned_file t_ofile = filename_to_ofile[(*it).c_str()];
		recipe_file << t_ofile.filename << " " << t_ofile.open_sha512_digest << " " << t_ofile.close_sha512_digest << " " << t_ofile.permissions << std::endl;
    }

    for (std::set<std::string>::iterator it=need_dirs.begin(); it!=need_dirs.end(); ++it){
        recipe_file << *it << " " << 0 << " " << 0 << " " << 0 << std::endl;
    }

	recipe_file.close();
    return 0;
}
