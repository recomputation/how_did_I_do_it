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

int wait_sysc(pid_t child) {
    int status;
    while (1) {
        ptrace(PTRACE_SYSCALL, child, 0, 0);
        waitpid(child, &status, 0);
        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80)
            return 0;
        if (WIFEXITED(status))
            return 1;
    }
}


int exec_trace(pid_t child, int conn_fd){

    int status;
    struct user_regs_struct regs;
    waitpid(child, &status, 0);

    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);

    while(1){
        if (wait_sysc(child) != 0) break;

        long answ = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG, NULL);
        int retval;
        int temp_fd;
        char* str;
		char* msg;

        switch(answ){
            case SYS_open:
                ptrace(PTRACE_GETREGS, child, NULL, &regs);
                str=read_string(child, get_arg(regs, 0));

                if (wait_sysc(child) != 0) break;
                retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG2, NULL);

				msg = format_msg(OPEN, str, retval);
                save_data(conn_fd, msg, strlen(msg));

				free(str);
				free(msg);
                break;
            case SYS_close:
                ptrace(PTRACE_GETREGS, child, NULL, &regs);
                temp_fd = get_arg(regs, 0);

                if (wait_sysc(child) != 0) break;
                retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG2, NULL);

                str = malloc( count_num(temp_fd) * sizeof(char) + 1 );
				sprintf(str, "%d", temp_fd);

				msg = format_msg(CLOSE, str, retval);
				save_data(conn_fd, msg, strlen(msg));
				free(str);

                break;
            case SYS_read:
                ptrace(PTRACE_GETREGS, child, NULL, &regs);
                temp_fd = get_arg(regs, 0);
                str=read_string(child, get_arg(regs, 1));
                printf("Read '%s' from '%d'\n", str, temp_fd);
                free(str);

                break;
            case SYS_write:
                ptrace(PTRACE_GETREGS, child, NULL, &regs);
                temp_fd= regs.rdi;
                str=read_string(child, get_arg(regs, 1));
                printf("Writing '%s' to '%d' ... ", str, temp_fd);
                free(str);
                if (wait_sysc(child) != 0) break;
                retval = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*REG2, NULL);

                if (retval == -1){
                    printf("FAIL\n");
                }else{
                    printf("%d written\n", retval);
                }
                break;
            default:
                break;
                //printf("The system call: %s(%ld)\n", decode_sc(answ), answ);
        }
    }
	return 0;
}

int trace(int argc, char **argv, int fd){
	if (argc < 2) {
        fprintf(stderr, "Usage: %s executable args\n", argv[0]);
        return 1;
    }

    pid_t child;
    child = fork();
    if(child == 0) {
		return exec_child(argc-1, argv+1);
    } else {
        return exec_trace(child, fd);
    }
}
