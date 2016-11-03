//
// Created by alin on 10/30/16.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "functionalities.h"
#include "constants.h"


void start_handling_commands();


int main() {
    start_handling_commands();
    return 0;
}

void start_handling_commands() {
    char command[COMM_LENGTH];
    char response[RESPONSE_LENGTH];
    int socket_pair[2], internal_pipe[2], pipe_fd;
    int pid;
    int command_length, response_length;

    //create the internal pipe for commands passing
    pipe(internal_pipe);

    //create socket pair for message length communication
    socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair);

    pid = fork();
    if (!pid) {
        //parent process
        while (1) {
            printf("SIPCPP\\:> ");
            fgets(command, COMM_LENGTH, stdin);
            command_length = strlen(command) + 1;

            write(socket_pair[1], &command_length, 4);
            write(internal_pipe[1], command, command_length);

            read(socket_pair[1], &response_length, 4);

            if (response_length == EXIT_STATUS)
                break;

            //wait for the actual file to be created
            while (access(FIFO_NAME, F_OK) == -1);
            pipe_fd = open(FIFO_NAME, O_RDONLY);

            read(pipe_fd, &response, response_length);

            printf("%s\n", response);
        }
    } else {
        //child process
        close(0);
        int is_logged_in = 0;
        while (1) {
            read(socket_pair[0], &command_length, 4);
            read(internal_pipe[0], command, command_length);

            /* handle the command here */
            //handle the login
            char *p = strtok(command, " \t\n");
            if (!strcmp(p, LOGIN)) {
                if (is_logged_in == 1) {
                    sprintf(response, "Another user is already logged in");
                    goto out; //NEVER USE GOTO
                }
                p = strtok(NULL, " \t\n");
                if (p == NULL)
                {
                    // LOGIN HAS NO NAME
                    sprintf(response,"Command format: login <username>");
                    goto out;
                }
                char name[ARG_LENGTH];
                strcpy(name, p);

                if (check_for_user(name) == 1) {
                    is_logged_in = 1;
                    sprintf(response, "Logged in as %s! Have fun!", name);
                } else {
                    sprintf(response, "Login failed, username \"%s\" doesn't exist", name);
                }

            } else if (!strcmp(p, MY_STAT)) {
                if (is_logged_in == 0) {
                    sprintf(response, "You are required to login before executing commands");
                    goto out; //NEVER USE GOTO
                }
                p = strtok(NULL, " \t\n");
                if (p == NULL)
                {
                    // LOGIN HAS NO NAME
                    sprintf(response,"Command format: mystat <path>");
                    goto out;
                }
                struct stat file_stats;
                int status = stat(p,&file_stats);
                if (status == -1)
                {
                    sprintf(response,"Error at stat: %s",strerror(errno));
                    goto out;
                }
                print_file_info(p,file_stats,response);
            }
            else if (!strcmp(p, HELP)) {
                sprintf(response, "Write with lower-case letters!");
            } else if (!strcmp(p, QUIT)) {
                response_length = EXIT_STATUS;
                write(socket_pair[0], &response_length, 4);
                close(socket_pair[0]);
                close(internal_pipe[0]);
                close(internal_pipe[1]);
                exit(0);
            } else {
                sprintf(response, "Invalid command, try \"help\" for assistance");
            }

            out:
            response_length = strlen(response) + 1;
            write(socket_pair[0], &response_length, 4);

            mkfifo(FIFO_NAME, 0666);
            pipe_fd = open(FIFO_NAME, O_WRONLY);
            write(pipe_fd, response, response_length);
        }
    }
}
