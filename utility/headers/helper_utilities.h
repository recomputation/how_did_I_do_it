#include <string>
#include <vector>

int count_num(int n);
int isDirectory(std::string path);
int should_track(std::string file_name);

std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

void folderize(std::string path, std::string tmp_dirname);

int copy_file(std::string orig_filename, std::string new_filename);
char* read_string(pid_t child, unsigned long addr);
