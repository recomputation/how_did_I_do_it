#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "../headers/communicator.h"
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/md5.h>

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

static char* base = "/tmp/ilia_";

char* file_md5_and_copy(char* filename){
	unsigned char c[MD5_DIGEST_LENGTH];

	FILE* inFile = fopen (filename, "rb");

	if (!inFile){
		printf("Failed to open the file");
		return 0;
	}

	//TODO: check if the file is accessible

    MD5_CTX mdContext;
    int bytes;
	int dataSize = 1024;
    char data[dataSize];

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, dataSize, inFile)) != 0){
        MD5_Update (&mdContext, data, bytes);
	}
    MD5_Final (c,&mdContext);

	//TODO: move to a separate config file or something
	int basesize = strlen(base);
	char* newfile = malloc(sizeof(char)*(MD5_DIGEST_LENGTH+basesize) + 2);
	char* index = malloc(sizeof(char)*(MD5_DIGEST_LENGTH+1));

	for(int i = 0; i < MD5_DIGEST_LENGTH; i++) sprintf(&index[i], "%02x", (unsigned int)c[i]);
	index[MD5_DIGEST_LENGTH]='\0';

	strcpy(newfile, base);
	strcat(newfile, index);

    FILE* toFile = fopen (newfile, "wb+");
	//TODO: add check for file access here
	rewind(inFile);

    ssize_t nread;
	while ((nread = fread(data, 1, dataSize, inFile)) > 0){
        char* out_ptr = data;
        ssize_t nwritten;
        do {
            nwritten = fwrite(out_ptr, 1, nread, toFile);
            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
        } while (nread > 0);
    }

	//TODO: NEED TO DO ERROR HANDLING HERE

	fclose(inFile);
	fclose(toFile);
	return newfile;
}

int isDirectory(char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

static char separator = '|';

FILE* initiate_communication(int argc, char** argv){
    return tmpfile();
}

int opened_file(FILE* conn, char* program_name, char* file_name, int did_create){
    // Method invoked when a file is openned.
    // Need to check if the file is not some sysfile and make a copy of the file here
    // Save it to the control file as well
    printf("Open %s by %s(%d)\n", file_name, program_name, did_create);
	if(!isDirectory(file_name))
		free(file_md5_and_copy(file_name));
    return 0;
}

int read_from_file(FILE* conn, char* program_name, char* file_name){
    // Method that is invoked when the read is perfromed from a particular file
    // That method is needed to find dependencies for the program
    printf("Read %s by %s\n", file_name, program_name);
    fputs("r", conn);
    fputs(file_name, conn);
    fputs("\n", conn);
    return 0;
}

int write_to_file(FILE* conn, char* program_name, char* file_name){
    // Method that is invoked when the write is happened to a particular file
    // That method is needed to find the files produced by particular programs
    // In here I need to store the file into the config and make it indexable for the finding
    printf("Write %s by %s\n", file_name, program_name);
    fputs("w", conn);
    fputs(file_name, conn);
    fputs("\n", conn);
    return 0;
}

int rename_file(FILE* conn, char* program_name, char* from, char* to){
	printf("Rename FROM:%s TO:%s by %s\n", from ,to, program_name);
	//TODO: maybe rename it here?
	fputs("a", conn);
	fputs(from, conn);
	fputs(";", conn);
	fputs(to, conn);
	fputs("\n", conn);
}

int file_close(FILE* conn, char* program_name, char* file_name){
    // Note if the file was closed without any changes (why was it open in the first place?)
    printf("Close %s by %s\n", file_name, program_name);
	//TODO: need to be carefyul here. It might be the case that the file is moved, without a closed handle
	//TODO: add the outlined check
	if(!isDirectory(file_name))
		free(file_md5_and_copy(file_name));
    return 0;
}

int should_track(char* file_name){
    if ( file_name[0] != '/' || (file_name[0] == '/' && strstr(file_name, getlogin()))){
        return 1;
    }
    return 0;
}

int close_communication(FILE* conn){
    // Need to process the file here to create the dependencies file
    char ch;
    rewind(conn);

    int MAX_FILES=1024;
    int MAX_FILENAME=150;
    char* read_files[MAX_FILES];
    char* wrote_files[MAX_FILES];
    int c_r=0, c_w=0, c_c=0;

    char type;
    char* c_filename;
    short new_word = 1;

    while(( ch = fgetc(conn) ) != EOF){
        if(new_word){
            type = ch;
            new_word = 0;
            c_c=0;
            c_filename = malloc( (MAX_FILENAME) * sizeof(char));
            continue;
        }
        if(ch == '\n'){
			c_filename[c_c] = '\0';
            new_word = 1;
            switch(type){
            case 'w':
                for(int i=0; i< c_w; i++){
                    int rc = strcmp(c_filename, wrote_files[i]);
                    if(!rc){
                        free(c_filename);
                        c_filename=NULL;
                        break;
                    }
                }
                if(c_filename){
                    wrote_files[c_w++]=c_filename;
                }
                break;
            case 'r':
                for(int i=0; i< c_r; i++){
                    int rc = strcmp(c_filename, read_files[i]);
                    if(!rc){
                        free(c_filename);
                        c_filename=NULL;
                        break;
                    }
                }
                if(c_filename){
                    read_files[c_r++]=c_filename;
                }
                break;
            }
            continue;
        }
        c_filename[c_c++]=ch;
    }

    printf("Writes:%d, Reads:%d\n", c_w, c_r);

	/*
    for(int i=0; i< c_w; i++){
        printf("WRITE %s\n", wrote_files[i]);
		char* index = file_md5_and_copy(wrote_files[i]);
		free(index);
    }

    for(int i=0; i< c_r; i++){
        printf("READ %s\n", read_files[i]);
		char* index = file_md5_and_copy(read_files[i]);
		free(index);
    }
	*/
    fclose(conn);
    return 0;
}

char* format_msg(event_type type, char *msg, int work_fd, int retvalue){
    char *result = malloc( count_num(type) + 1 + strlen(msg) + 1 + count_num(work_fd) + 1 + count_num(retvalue) + 1);

	sprintf(result, "%d%c%d%c%d%c%s", type, separator, retvalue, separator, work_fd, separator, msg);
    return result;
}


