// File with the utilities used in the programs

#include <limits.h>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <string.h>
#include <pwd.h>

#include <iostream>
#include <fstream>

//Method used to give number or characters taken by a particular number
//After trying out different methods to calculate it this one has proven to be the fastest
int count_num (int n) {
    if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    return 10;
}

//Method checks if given path is a directory
int isDirectory(std::string path) {
   struct stat statbuf;
   if (stat(path.c_str(), &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

int should_track(std::string file_name){
    //TODO: maybe add a set of rules here or something?

    struct passwd* gl = getpwuid(getuid());
    if ( file_name[0] != '/' || (file_name[0] == '/' && file_name.find(gl->pw_name) != std::string::npos)){
        return 1;
    }
    return 0;
}

void folderize(std::string path, std::string tmp_dirname){

    std::vector<std::string> sv = split(path, '/');
    struct stat st;
    std::string curr = std::string(tmp_dirname);

    //Maybe do something about copying the permissions
    for(std::vector<std::string>::size_type i = 0; i != sv.size()-1; i++) {
        curr += "/" + sv[i];
        if (stat(curr.c_str(), &st) == -1 ){
            mkdir(curr.c_str(), 0700);
        }
    }
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

char* read_string(pid_t child, unsigned long addr) {
    char *val = (char*) malloc(4096);
    int allocated = 4096;
    int read = 0;
    unsigned long tmp;
    while (1) {
        // I feel pretty bad when do that
        if (read + (int)(sizeof(tmp)) > allocated) {
            allocated *= 2;
            val = (char*) realloc(val, allocated);
        }
        tmp = ptrace(PTRACE_PEEKDATA, child, addr + read);
        if(errno != 0) {
            val[read] = 0;
            break;
        }
        memcpy(val + read, &tmp, sizeof(tmp));
        if (memchr(&tmp, 0, sizeof(tmp)) != NULL)
            break;
        read += sizeof(tmp);
    }
    return val;
}


