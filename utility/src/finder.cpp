#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>

#include <unistd.h>
#include <fcntl.h>

#include "../headers/communicator.h"
#include "../headers/finder.h"
#include "../headers/helper_utilities.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

extern std::set<std::string> files_read;
extern std::set<std::string> files_written;

static std::set<std::string> expanded;
static std::unordered_map<std::string, std::unordered_map<std::string, bool>> executed_cmds;

bool have_recipe(std::string sha512_digest){
	DIR* d;
    std::string newfile = recipe_directory + sha512_digest;
  	d = opendir(newfile.c_str());
    if (d){
        closedir(d);
        return true;
    }
    return false;
}


int find_recipe_by_sha512(std::string sha512_digest){

    expanded.insert(std::string(sha512_digest));
    std::string newfile = recipe_directory + sha512_digest;

    std::cout << "#Recipe: " << newfile << std::endl;
    std::string me = std::string(".");
    std::string up_me = std::string("..");

	DIR* d;
  	struct dirent *dir;
  	d = opendir(newfile.c_str());

  	if (d){
    	while ((dir = readdir(d)) != NULL){
			if (me.compare(std::string(dir->d_name)) && up_me.compare(std::string(dir->d_name))){
                std::string file_name = newfile + "/" + std::string(dir->d_name);

                std::ifstream t_file (file_name.c_str());

				if (!t_file.is_open()){
					continue;
				}

                std::string orig_filename;
                std::string cwd;
                std::string command;
                std::string line;

                getline(t_file, orig_filename); // Original filename
                getline(t_file, cwd);
                getline(t_file, command); // Command used to generate

                std::cout << "#Building: " << orig_filename << std::endl << command << std::endl;
                std::cout << "#CWD:" << cwd << std::endl;

                //All of the dependencies
                while (getline(t_file, line)){
                    std::vector<std::string> s_line = split(line, ' ');
                    std::string need_file = s_line[1];

                    if (expanded.find(need_file) == expanded.end()){
                        if (have_recipe(need_file)){
                            find_recipe_by_sha512(need_file);
                        }else{
                            expanded.insert(need_file);
                            std::cout << "Need to copy: " << s_line[0] << std::endl;
                        }
                    }
                }
                std::cout << std::endl;
			}
		}
		closedir(d);
		return 0;
    }else{
        std::cout << "No recipe found" << std::endl;
        return -1;
    }
}

int find_recipe_by_name(std::string filename){
	char* sha512_of_file = get_sha512(filename);

	if(!sha512_of_file){
		return -1;
	}

	return find_recipe_by_sha512(std::string(sha512_of_file));
}

int build_recipe(std::string sha512_digest, std::string tmp_dirname, std::ofstream& execer){
    expanded.insert(std::string(sha512_digest));
    std::string newfile = recipe_directory + sha512_digest;

    std::string me = std::string(".");
    std::string up_me = std::string("..");

	DIR* d;
  	struct dirent *dir;
  	d = opendir(newfile.c_str());

  	if (d){
    	while ((dir = readdir(d)) != NULL){
			if (me.compare(std::string(dir->d_name)) && up_me.compare(std::string(dir->d_name))){
                std::string file_name = newfile + "/" + std::string(dir->d_name);

                std::ifstream t_file (file_name.c_str());

				if (!t_file.is_open()){
					continue;
				}

                std::string orig_filename;
                std::string cwd;
                std::string permissions;
                std::string command;
                std::string line;

                getline(t_file, orig_filename); // Original filename
                //TODO: the issue is if the file is dependent on the absence of folders we might screw it up
                //TODO: Maybe add the permissions copying to the folders as well
                folderize(orig_filename, tmp_dirname);

                getline(t_file, cwd);
                getline(t_file, command); // Command used to generate

                while (getline(t_file, line)){
                    std::vector<std::string> s_line = split(line, ' ');
                    std::string need_file = s_line[1];
                    std::string need_file_sha = s_line[2];
                    std::string permissions = s_line[3];

                    if (expanded.find(need_file) == expanded.end()){
                        if (have_recipe(need_file)){
                            build_recipe(need_file, tmp_dirname, execer);
                        }else{
                            expanded.insert(need_file);
                            folderize(s_line[0], tmp_dirname);
                            if (need_file_sha.compare("0")!=0){
                                std::string from = file_directory + need_file_sha + std::string(dir->d_name);
                                std::string to = tmp_dirname + s_line[0];

                                int f = open(to.c_str(), O_CREAT, atoi(permissions.c_str()));
                                if(f > 0){
                                    close(f);
                                }

                                copy_file(from, to);
                            }
                       }
                    }
                    std::string real_path = tmp_dirname + cwd;
                    if (executed_cmds[real_path].find(command) == executed_cmds[real_path].end()){
                        execer << "cd" << real_path << std::endl << command << std::endl;
                        executed_cmds[real_path][command] = true;
                    }
                }
			}
		}
		closedir(d);
		return 0;
    }else{
        std::cout << "No recipe found" << std::endl;
        return -1;
    }
}

int build_environment(std::string filename){
    char s_template[] = "/tmp/saint-lication.XXXXXX";
    char* tmp_dirname = mkdtemp (s_template);
    std::string exec_me = std::string(tmp_dirname) + "/saintplication.sh";

    std::cout << "The environment is replicated here: " << std::string(tmp_dirname) << std::endl;

    char* sha512_of_file = get_sha512(filename);

    if (!sha512_of_file){
        return -1;
    }

    int f = open(exec_me.c_str(), O_CREAT, 0700);
    if (f > 0){
        close(f);
    }

    std::ofstream execer (exec_me.c_str());

    build_recipe(std::string(sha512_of_file), std::string(tmp_dirname), execer);
    delete sha512_of_file;

    return 0;
}
