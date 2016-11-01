//
// Created by alin on 11/1/16.
//

#include <bits/stat.h>
#include "functionalities.h"


int check_for_user(const char* name)
{
    char user_name[100];
    FILE* user_file = fopen(USERS_FILE,"r");
    while (!feof(user_file))
    {
        fscanf(user_file,"%s",user_name);
        if (!strcmp(user_name,name))
        {
            fclose(user_file);
            return 1;
        }
    }
    fclose(user_file);
    return 0;
}

void print_file_info(const char *path, const struct stat file_stats, char *buffer) {

    char row[MAX_COL];
    sprintf(row,"Information about %s :\n",path);
    buffer[0] = '\0';
    strcat(buffer,row);
    sprintf(row, "Size of the file (bytes): %d\n",file_stats.st_size);
    strcat(buffer,row);
    sprintf(row, "Size of the file (bytes): %d\n",file_stats.st_size);

    return;
}
