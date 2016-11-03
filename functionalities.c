//
// Created by alin on 11/1/16.
//

#include <time.h>
#include <sys/stat.h>
#include <stdlib.h>
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
    strcat(buffer,row);
    sprintf(row, "Size of the file (bytes): %d\n",file_stats.st_size);
    strcat(buffer,row);
    sprintf(row, "Last access time: %s",ctime(&(file_stats.st_atime)));
    strcat(buffer,row);
    sprintf(row, "Last modification time: %s",ctime(&(file_stats.st_mtime)));
    strcat(buffer,row);
    sprintf(row, "Permissions : ");
    strcat(buffer,row);
    generate_human_readable_perm(file_stats,buffer);
    return;
}

char* generate_human_readable_perm(const struct stat file_stat,char *buff)
{
    // courtesy of http://codewiki.wikidot.com/c:system-calls:stat

    strcat(buff, (S_ISDIR(file_stat.st_mode)) ? "d" : "-");
    strcat(buff, (file_stat.st_mode & S_IRUSR) ? "r" : "-");
    strcat(buff, (file_stat.st_mode & S_IWUSR) ? "w" : "-");
    strcat(buff, (file_stat.st_mode & S_IXUSR) ? "x" : "-");
    strcat(buff, (file_stat.st_mode & S_IRGRP) ? "r" : "-");
    strcat(buff, (file_stat.st_mode & S_IWGRP) ? "w" : "-");
    strcat(buff, (file_stat.st_mode & S_IXGRP) ? "x" : "-");
    strcat(buff, (file_stat.st_mode & S_IROTH) ? "r" : "-");
    strcat(buff, (file_stat.st_mode & S_IWOTH) ? "w" : "-");
    strcat(buff, (file_stat.st_mode & S_IXOTH) ? "x" : "-");
    strcat(buff, "\n");
}

int is_dir(const char* path)
{
    DIR* d;
    d = opendir(path);
    if (d == NULL)
        return 0;
    closedir(d);
    return 1;
}

void find_file(const char* dir_path, const char* filename, char* buffer)
{
    struct DIR* dir;
    dir = opendir(dir_path);
    if (dir == NULL) return;
    struct dirent *entry;
    char new_path[PATH_MAX];
    while ((entry = readdir(dir)) != NULL)
    {
        if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
            continue;
        sprintf(new_path,"%s/%s",dir_path,entry->d_name);
        if (entry->d_type != DT_DIR)
        {
            if (!strcmp(entry->d_name,filename))
            {
                strcat(strcat(strcat(buffer,"Found the file at : "),new_path),"\n");
                struct stat file_stats;
                stat(new_path,&file_stats);
                print_file_info(new_path,file_stats,buffer);
                strcat(buffer,"\n");
            }
        }
        else
            find_file(new_path,filename,buffer);
    }
    closedir(dir);
    return;
}