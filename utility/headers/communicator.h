// Header file for the functions that should be defined in the communication interface

typedef enum {OPEN, CLOSE, READ, WRITE} event_type;

FILE* initiate_communication(int argc, char** argv);
int save_data(int fd, char* data, size_t size);
int close_communication(FILE* fd);

char* format_msg(event_type type, char *msg, int work_fd, int retvalue);

int count_num (int n);

// Method is invoked when the file is openned
int opened_file(FILE* conn, char* program_name, char* filename, int did_create);

// Method is invoked when read from a file is perfromed
int read_from_file(FILE* conn, char* program_name, char* file_name);

// Method is invoked when a write is perfomed on the file
int write_to_file(FILE* conn, char* program_name, char* file_name);

// Method is invoked when the close is invoked of a particular file from the program
int file_close(FILE* conn, char* prgram_name, char* file_name);

// Method is invoked to check if the file should be tracked
// 0 if should not
// 1 if should
int should_track(char* file_name);

