// Header file for the functions that should be defined in the communication interface

typedef enum {OPEN, CLOSE, READ, WRITE} event_type;

int initiate_communication(char* link);
int save_data(int fd, char* data, size_t size);
int close_communication(int fd);

char* format_msg(event_type type, char *msg, int retvalue);

int count_num (int n);
