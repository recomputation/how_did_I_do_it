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

#if __WORDSIZE == 64
#define REG ORIG_RAX
#define REG2 RAX
#else
#define REG ORIG_EAX
#define REG2 EAX
#endif

int exec_child(int argc, char **argv){
	char *args [argc+1];
    memcpy(args, argv, argc * sizeof(char*));
    args[argc] = NULL;
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    kill(getpid(), SIGSTOP);
    return execvp(args[0], args);
}

char *read_string(pid_t child, unsigned long addr) {
    char *val = malloc(4096);
    int allocated = 4096;
    int read = 0;
    unsigned long tmp;
    while (1) {
        // I feel pretty bad when do that
        if (read + (int)(sizeof(tmp)) > allocated) {
            allocated *= 2;
            val = realloc(val, allocated);
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

typedef enum {EXITTED, STOPPED, FORKED} ptevent_type;

ptevent_type wait_sysc(pid_t child) {
    int status;
    while (1) {
        waitpid(-1, &status, __WALL);

        if (status>>8 == (SIGTRAP | (PTRACE_EVENT_VFORK<<8))){
            long newpid;
            ptrace(PTRACE_GETEVENTMSG, child, NULL, (long) &newpid);
            ptrace(PTRACE_SYSCALL, newpid, NULL, NULL);
            return FORKED;
        }

        if (status >> 16 == PTRACE_EVENT_FORK) {
            long newpid;
            ptrace(PTRACE_GETEVENTMSG, child, NULL, (long) &newpid);
            ptrace(PTRACE_SYSCALL, newpid, NULL, NULL);
            return FORKED;
        }

        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80)
            return STOPPED;
        if (WIFEXITED(status))
            return FORKED;
    }
}


// Child and the program name
int exec_trace(pid_t child, char* pn, FILE* conn){

    int status;
    struct user_regs_struct regs;
    waitpid(child, &status, 0);

    char* descriptiors_to_filename[20]={NULL};
    ptrace(PTRACE_ATTACH, child, NULL, NULL);
    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK); //)| PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEVFORKDONE | PTRACE_O_TRACEEXIT);

    struct stat openfile;
    int retval;
    int temp_fd;
    char* str;
    int filecreate = 0 ;
    int num_proc = 1;

    while(1){
        while(1){
            ptrace(PTRACE_SYSCALL, child, 0, 0);
            child = waitpid(-1, &status, __WALL);
            if (status >> 16 == PTRACE_EVENT_FORK) {
                long newpid;
                ptrace(PTRACE_GETEVENTMSG, child, NULL, (long) &newpid);
                ptrace(PTRACE_SYSCALL, newpid, NULL, NULL);
                num_proc++;
                continue;
            }
            if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80){
                break;
            }

            if (WIFEXITED(status)){
                num_proc--;
                break;
            }
        }

        if(num_proc <= 0){
            break;
        }

        long answ = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG, NULL);
        switch(answ){
            case SYS_open:
                ptrace(PTRACE_GETREGS, child, NULL, &regs);
                str=read_string(child, get_arg(regs, 0));

                // Doing detection of the file
                if (stat(str, &openfile)< 0){
                    filecreate = 1;
                }else{
                    filecreate = 0;
                }

                ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                waitpid(child, &status, 0);
                retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG2, NULL);

                if (retval > 0 && should_track(str) ){
                    descriptiors_to_filename[retval] = str;
                    opened_file(conn, pn, str, filecreate);
                }
                break;

            case SYS_close:
                ptrace(PTRACE_GETREGS, child, NULL, &regs);
                temp_fd = get_arg(regs, 0);

                ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                waitpid(child, &status, 0);
                retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG2, NULL);

                if (descriptiors_to_filename[temp_fd]){
                    file_close(conn, pn, descriptiors_to_filename[temp_fd]);
                    free(descriptiors_to_filename[temp_fd]);
                    descriptiors_to_filename[temp_fd]=NULL;
                }
                break;
            case SYS_read:
                ptrace(PTRACE_GETREGS, child, NULL, &regs);
                temp_fd = get_arg(regs, 0);
                //str="";//read_string(child, get_arg(regs, 1));

                ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                waitpid(child, &status, 0);
                retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG2, NULL);

                if (descriptiors_to_filename[temp_fd]){
                    read_from_file(conn, pn, descriptiors_to_filename[temp_fd]);
                }
                break;
            case SYS_write:
                ptrace(PTRACE_GETREGS, child, NULL, &regs);
                temp_fd= get_arg(regs, 0);
                //str=read_string(child, get_arg(regs, 1));

                ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                waitpid(child, &status, 0);
                retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG2, NULL);

                if (descriptiors_to_filename[temp_fd]){
                    write_to_file(conn, pn, descriptiors_to_filename[temp_fd]);
                }
                break;
            /*case SYS_rename:
                ptrace(PTRACE_GETREGS, child, NULL, &regs);
                str = read_string(child, get_arg(regs,0));
                printf("RENAME FROM %s to ", str);
                free(str);
                str = read_string(child, get_arg(regs,1));
                printf("%s\n", str);
                free(str);
                ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                waitpid(child, &status, 0);
                break;*/
            default:
                //printf("The system call: %s(%ld)\n", decode_sc(answ), answ);
                ptrace(PTRACE_SYSCALL, child, 0, 0);
                break;
        }
    }
	return 0;
}

int trace(int argc, char **argv, char* pn, FILE* conn){
	if (argc < 2) {
        fprintf(stderr, "Usage: %s executable args\n", argv[0]);
        return 1;
    }

    pid_t child;
    child = fork();
    if(child == 0) {
		return exec_child(argc-1, argv+1);
    } else {
        return exec_trace(child, pn, conn);
    }
}
