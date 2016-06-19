// Header file for the Tracer functionality

#include <sys/ptrace.h>

using namespace std;
static const int ptrace_options = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEVFORK | PTRACE_O_TRACEEXIT;

int trace(int argc, char** argv, char* start_pwd, bool verbose);

int on_read(pid_t child, bool verbose);
int on_write(pid_t child, bool verbose);
int on_open(pid_t child, bool verbose);
int on_close(pid_t child, bool verbose);
int on_rename(pid_t child, bool verbose);
int on_chdir(pid_t child, bool verbose);
int on_execve(pid_t child, bool verbose);
int on_dup(pid_t child, bool verbose);
