#include <string>
#include <vector>

int count_num(int n);
int isDirectory(std::string path);
int should_track(std::string file_name);

std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

