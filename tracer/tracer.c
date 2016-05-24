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
#include "syscall_decoder.h"

#if __WORDSIZE == 64
#define REG ORIG_RAX
#else
#define REG ORIG_EAX
#endif

int exec_child(int argc, char **argv){
	char *args [argc+1];
    memcpy(args, argv, argc * sizeof(char*));
    args[argc] = NULL;
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    kill(getpid(), SIGSTOP);
    return execvp(args[0], args);
}

int main(int argc, char **argv){
	if (argc < 2) {
        fprintf(stderr, "Usage: %s executable args\n", argv[0]);
        exit(1);
    }

    pid_t child;
    child = fork();
    if(child == 0) {
		return exec_child(argc-1, argv+1);
    }
    else {
		int status;
		while(1){
			wait(&status);
			if(WIFEXITED(status)) break;

			long answ = ptrace(PTRACE_PEEKUSER, child, 8*REG, NULL);

			printf("The system call: %s(%ld)\n", decode_sc(answ), answ);
			ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		}
    }
    return 0;
}
