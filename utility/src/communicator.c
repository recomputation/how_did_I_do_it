#include <stdio.h>
#include <stdlib.h>
#include "../headers/communicator.h"
#include <limits.h>
#include <string.h>

int count_num (int n) {
    if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    return 10;
}

static char separator = '|';

int initiate_communication(char* link){
    printf("Starting the communication\n");

    return 0;
}

int save_data(int fd, char* data, size_t size){
    printf("%s\n", data);
    return 0;
}

int close_communication(int fd){
    printf("Closing the communicator\n");
    return 0;
}

char* format_msg(event_type type, char *msg, int retvalue){
    char *result = malloc( count_num(type) + 1 + strlen(msg) + 1 + count_num(retvalue) + 1);

	sprintf(result, "%d%c%d%c%s", type, separator, retvalue, separator, msg);
    return result;
}


