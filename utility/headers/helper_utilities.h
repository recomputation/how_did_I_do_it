#include <string>
#include <vector>

int count_num(int n);
int isDirectory(std::string path);
int should_track(std::string file_name);
void load_file_configs();

std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

void folderize(std::string path, std::string tmp_dirname);

std::string sha512_string(std::string gstr);
int copy_file(std::string orig_filename, std::string new_filename);
char* read_string(pid_t child, unsigned long addr);
