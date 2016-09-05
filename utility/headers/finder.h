#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <vector>

extern std::set<std::string> files_read;
extern std::set<std::string> files_written;

extern std::string file_directory;
extern std::string recipe_directory;

static std::set<std::string> expanded;
static std::unordered_map<std::string, std::unordered_map<std::string, bool>> executed_cmds;

int find_recipe_by_sha512(std::string sha512_digest);
int find_recipe_by_name(std::string filename);
int build_environment(std::string filename);
int build_env(std::string sha512_digest);
