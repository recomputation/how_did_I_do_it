// Header file for the functions that should be defined in the communication interface

int initiate_communication(argc, char** argv);
int save_data(int fd, char* data, size_t size);
int close_communication(int fd);
