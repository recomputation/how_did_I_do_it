#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <limits.h>
#include <sys/stat.h>

#include "../headers/tracer.h"
#include "../headers/syscall_decoder.h"
#include "../headers/communicator.h"
#include "../headers/helper_utilities.h"

#include <string>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#if __WORDSIZE == 64
#define REG ORIG_RAX
#define REG2 RAX
#else
#define REG ORIG_EAX
#define REG2 EAX
#endif

#define is_fork(s) ((s>>8)==(SIGTRAP | PTRACE_EVENT_FORK << 8))
#define is_vfork(s) ((s>>8)==(SIGTRAP | PTRACE_EVENT_VFORK << 8))
#define is_clone(s) ((s>>8)==(SIGTRAP | PTRACE_EVENT_CLONE << 8))

extern const int ptrace_options;

static unordered_map<int, std::unordered_map<int, std::string>> pid_to_descriptors_to_filename;
static unordered_map<int, std::string> pid_to_cwd;

int on_open(pid_t child, bool verbose){
    struct user_regs_struct regs;
    std::string path_s;

    ptrace(PTRACE_GETREGS, child, NULL, &regs);
    char* str=read_string(child, get_arg(regs, 0));

    if(str[0] == '/' && verbose){
        std::cout << "WARNING: ABSOLUTE PATH FOR THE FILE MAY CAUSE ERRORS: " << std::string(str) << std::endl;
    }

    std::string t_path;
    if (pid_to_cwd.find(child) != pid_to_cwd.end() && str[0] != '/'){
        t_path = pid_to_cwd[child] + "/" + std::string(str);
    }else{
        t_path = std::string(str);
    }

    char* rp=realpath(t_path.c_str(), NULL);
    if (!rp){
        path_s = std::string(t_path);
    }else{
        path_s = std::string(rp);
    }
    free(rp);

    bool filecreate;
    struct stat openfile;

    if (stat(path_s.c_str(), &openfile)< 0){
        filecreate = true;
    }else{
        filecreate = false;
    }

    int status;
    ptrace(PTRACE_SYSCALL, child, NULL, NULL);

    waitpid(child, &status, 0);
    int retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG2, NULL);

    if (retval > 0 && should_track(str)){
        if(!opened_file(path_s, filecreate)){
            if (verbose){
                cout << "[" << child << "] OPEN " << endl;
                cout << "\tpath: " << path_s << endl;
                cout << "\tTRACKING..." << endl;
            }
            pid_to_descriptors_to_filename[child][retval] = path_s;
        }else{
            std::cout << "\tFailed to open: " << path_s << std::endl;
        }
    }

    free(str);
    return 0;
}

int on_close(pid_t child, bool verbose){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child, NULL, &regs);

    int temp_fd = get_arg(regs, 0), status;
    ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    waitpid(child, &status, 0);

    if (pid_to_descriptors_to_filename[child].find(temp_fd) != pid_to_descriptors_to_filename[child].end()){
        if(verbose){
            cout << "[" << child << "] CLOSE\n\tpath: " << pid_to_descriptors_to_filename[child][temp_fd] << endl;
        }
        file_close(pid_to_descriptors_to_filename[child][temp_fd]);
        pid_to_descriptors_to_filename[child].erase(temp_fd);
    }
    return 0;
}

int on_dup(pid_t child, bool verbose){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child, NULL, &regs);
    int from = get_arg(regs,0);
    int to = get_arg(regs,1);
    if(verbose){
        std::cout << "[" << child << "] DUP2: " << from << " " << to << std::endl;
    }

    if(pid_to_descriptors_to_filename[child].find(from) != pid_to_descriptors_to_filename[child].end()){
        pid_to_descriptors_to_filename[child][to] = std::string(pid_to_descriptors_to_filename[child][from]);
    }
    return 0;
}

int on_read(pid_t child, bool verbose){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child, NULL, &regs);
    int temp_fd = get_arg(regs, 0);

    char* str=read_string(child, get_arg(regs, 0));
    ptrace(PTRACE_SYSCALL, child, NULL, NULL);

    if (pid_to_descriptors_to_filename[child].find(temp_fd) != pid_to_descriptors_to_filename[child].end()){
        if (verbose){
            std::cout << "[" << child << "] READ\n\tpath: " << pid_to_descriptors_to_filename[child][temp_fd] << "\n\tstring: " << str << std::endl;
        }
        read_from_file(pid_to_descriptors_to_filename[child][temp_fd]);
    }

    free(str);
    return 0;
}

int on_execve(pid_t child, bool verbose){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child, NULL, &regs);
    char* str = read_string(child, get_arg(regs, 0));

    if (verbose){
        std::cout << "[" << child << "] EXECVE\n\tcmd: " << str << std::endl;
    }

    free(str);

    return 0;
}

int on_write(pid_t child, bool verbose){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child, NULL, &regs);
    int temp_fd= get_arg(regs, 0);

    char* str=read_string(child, get_arg(regs, 1));
    ptrace(PTRACE_SYSCALL, child, NULL, NULL);

    if (pid_to_descriptors_to_filename[child].find(temp_fd) != pid_to_descriptors_to_filename[child].end()){
        write_to_file(pid_to_descriptors_to_filename[child][temp_fd]);

        if (verbose){
            std::cout << "[" << child << "] WRITE\n\tpath: " << pid_to_descriptors_to_filename[child][temp_fd] << "\n\tstring: " << str << std::endl;
        }
    }

    free(str);
    return 0;
}

int on_rename(pid_t child, bool verbose){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child, NULL, &regs);
    char* str = read_string(child, get_arg(regs,0));
    char* to = read_string(child, get_arg(regs,1));

    string re_from = string(str);
    string re_to = string(to);

    if (pid_to_cwd.find(child) != pid_to_cwd.end() && re_from[0] != '/'){
        re_from= pid_to_cwd[child] + "/" + re_from;
        char* rr = realpath(re_from.c_str(), NULL);
        if (rr){
            re_from = std::string(rr);
        }
        free(rr);
    }

    if (pid_to_cwd.find(child) != pid_to_cwd.end() && re_to[0] != '/'){
        re_to= pid_to_cwd[child] + "/" + re_to;
        char* rr = realpath(re_to.c_str(), NULL);
        if (rr){
            re_to = std::string(rr);
        }
        free(rr);
    }

    if (verbose){
        std::cout << "[" << child << "] RENAME\n\tfrom: " << re_from << "\n\tto: " << re_to << std::endl;
    }

    rename_file(re_from, re_to);

    free(str);
    free(to);
    return 0;
}

int on_chdir(pid_t child, bool verbose){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child, NULL, &regs);
    char* str = read_string(child, get_arg(regs,0));

    pid_to_cwd[child] = std::string(str);

    if (verbose){
        std::cout << "Change directory to(" << child << "): " << str << std::endl;
    }

    free(str);
    return 0;
}

int exec_child(int argc, char **argv){
	char *args [argc+1];
    memcpy(args, argv, argc * sizeof(char*));
    args[argc] = NULL;
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    kill(getpid(), SIGSTOP);
    return execvp(args[0], args);
}

// Child and the program name
int exec_trace(pid_t child, char* start_pwd, bool verbose){

    int status;
    waitpid(child, &status, 0);

    pid_to_cwd[child] = std::string(start_pwd);

    ptrace(PTRACE_ATTACH, child, NULL, NULL);
    ptrace(PTRACE_SETOPTIONS, child, 0, ptrace_options);

    int num_proc = 1;

    while(1){
        while(1){
            ptrace(PTRACE_SYSCALL, child, 0, 0);
            child = waitpid(-1, &status, __WALL);

            if (is_fork(status) || is_vfork(status) || is_clone(status)){
                long newpid;
                ptrace(PTRACE_GETEVENTMSG, child, NULL, (long) &newpid);
                ptrace(PTRACE_ATTACH, newpid, NULL, NULL);
                ptrace(PTRACE_SYSCALL, newpid, NULL, NULL);
                ptrace(PTRACE_SETOPTIONS, newpid, 0, ptrace_options);

                pid_to_cwd[newpid] = pid_to_cwd[child];
                num_proc++;
                if(verbose){
                    std::cout << "[" << child << "] NEW: " << newpid << "\n\tProcesses running: "<< num_proc << std::endl;
                }
            }

            if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80){
                break;
            }

            if (WIFEXITED(status)){
                num_proc--;
                if (verbose){
                    std::cout << "[" << child << "] EXIT " << std::endl;
                }
                pid_to_descriptors_to_filename.erase(child);
                if (pid_to_cwd.find(child) != pid_to_cwd.end()){
                    pid_to_cwd.erase(child);
                }
                break;
            }
        }

        if(num_proc <= 0){
            break;
        }

        long answ = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG, NULL);
        switch(answ){
            case SYS_execve: on_execve(child, verbose); break;
            case SYS_open: on_open(child, verbose); break;
            case SYS_close: on_close(child, verbose); break;
            case SYS_read: on_read(child, verbose); break;
            case SYS_write: on_write(child, verbose); break;
            case SYS_rename: on_rename(child, verbose); break;
            case SYS_chdir: on_chdir(child, verbose); break;
            case SYS_dup2: on_dup(child, verbose); break;
            default: break;
        }
    }

    pid_to_cwd.clear();
    pid_to_descriptors_to_filename.clear();
	return 0;
}

int trace(int argc, char **argv, char* start_pwd, bool verbose){
    pid_t child;
    child = fork();
    if(child == 0) {
		return exec_child(argc, argv);
    } else {
        return exec_trace(child, start_pwd, verbose);
    }
}
