// Header file for the functions that should be defined in the communication interface
#include <string>
#include <iostream>
#include <set>

static std::string file_directory("/tmp/files_fd");
static std::string recipe_directory("/tmp/recipes_fd");

class openned_file{
	public:
		std::string filename;
		std::string open_sha512_digest;
		std::string close_sha512_digest;
		int permissions;
		bool written;
		bool read;
		bool created;
		bool closed;
};

struct ofile_compare {
    bool operator() (openned_file lhs, openned_file rhs) const{
        return lhs.filename < rhs.filename;
    }
};

struct set_string_compare{
  bool operator()(std::string* lhs, std::string* rhs) const
  {
    return *lhs < *rhs;
  }
};

struct pointer_deleter{
    template <typename T> void operator () (T *ptr){
	    if(ptr){
    	    delete ptr;
        }
    }
};

template <typename T> void deallocate_mapsecond(T c){
    for (typename T::iterator i = c.begin(); i != c.end(); ++i){
        c.erase(i);
    }
}

template <typename T> void deallocate_container(T c){
    for (typename T::iterator i = c.begin(); i != c.end(); ++i){
        if (*i){
            delete *i;
        }
    }
}

int close_communication(std::string program_name);
int count_num (int n);

void initiate_communication(char* cwd);

// Method is invoked when the file is openned
int opened_file(std::string filename, bool did_create);

// Method is invoked when read from a file is perfromed
int read_from_file(std::string file_name);

// Method is invoked when a write is perfomed on the file
int write_to_file(std::string file_name);

// Method is invoked when the close is invoked of a particular file from the program
int file_close(std::string file_name);

int rename_file(std::string from, std::string to);

// Method is invoked to check if the file should be tracked
int should_track(std::string file_name);

char* get_sha512(std::string filename);
std::string* file_sha512_and_copy(std::string filename);

int write_recipe(std::string filename, std::string sha512_digest, std::string program_name);
