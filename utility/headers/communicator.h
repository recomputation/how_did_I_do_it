// Header file for the functions that should be defined in the communication interface
#include <string>
#include <set>

static const std::string file_directory("/tmp/ilia_fd/");
static const std::string recipe_directory("/tmp/ilia_recipes/");

int save_data(int fd, char* data, size_t size);
int close_communication(char* file_name);
int count_num (int n);

void initiate_communication();

// Method is invoked when the file is openned
int opened_file(char* program_name, char* filename, int did_create);

// Method is invoked when read from a file is perfromed
int read_from_file(char* program_name, char* file_name);

// Method is invoked when a write is perfomed on the file
int write_to_file(char* program_name, char* file_name);

// Method is invoked when the close is invoked of a particular file from the program
int file_close(char* prgram_name, char* file_name);

int rename_file(char* program_name, char* from, char* to);

// Method is invoked to check if the file should be tracked
// 0 if should not
// 1 if should
int should_track(char* file_name);

char* get_sha512(std::string filename);
char* file_sha512_and_copy(std::string filename);

int write_recipe(std::string filename, char* sha512_digest, char* program_name, std::set<std::string> read_files);
