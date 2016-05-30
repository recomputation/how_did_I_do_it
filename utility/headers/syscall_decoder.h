#include <sys/types.h>
#include <sys/user.h>
#include <string>

std::string decode_sc(long call);
long get_arg(struct user_regs_struct regs, int which);
