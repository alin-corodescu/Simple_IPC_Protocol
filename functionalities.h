//
// Created by alin on 11/1/16.
//
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "constants.h"


int check_for_user(const char* name);

void print_file_info(const char *path, const struct stat file_stats, char *buffer);

char* generate_human_readable_perm(const struct stat file_stat,char *buff);

int is_dir(const char* path);

void find_file(const char* dir_path, const char* filename, char* buffer);

