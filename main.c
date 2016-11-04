//
// Created by alin on 10/30/16.
//
#include "functionalities.h"
void start_handling_commands();


int main() {
    start_handling_commands();
    return 0;
}

void start_handling_commands() {
    int status;
    char command[COMM_LENGTH];
    char response[RESPONSE_LENGTH];
    int socket_pair[2], internal_pipe[2], pipe_fd;
    int pid;
    int command_length, response_length;

    //create the internal pipe for commands passing
    status  = pipe(internal_pipe);
    if (status == -1)
    {
        printf("Error at internal pipe creation: %s",strerror(errno));
        exit(ERR_FILE);
    }
    //create socket pair for message length communication
    status = socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair);
    if (status == -1)
    {
        printf("Error at socketpair creation: %s\n",strerror(errno));
        exit(ERR_FILE);
    }

    pid = fork();
    if (pid == -1)
    {
        printf("Error at creating new child process %s\n",strerror(errno));
        exit(ERR_PROC);
    }
    if (!pid) {
        //parent process
        while (1) {
            printf("SIPCPP\\:> ");
            fgets(command, COMM_LENGTH, stdin);
            command_length = strlen(command) + 1;

            status = write(socket_pair[1], &command_length, 4);
            if (status < 4)
            {
                printf("[parent] Error at writing the command length %s\n",strerror(errno));
                close(socket_pair[0]); close(socket_pair[1]); close(internal_pipe[0]); close(internal_pipe[1]);
                kill(pid,SIGKILL);
                exit(ERR_COMM);
            }
            status = write(internal_pipe[1], command, command_length);
            if (status < command_length)
            {
                printf("[parent] Error at writing the command in the internal pipe : %s\n",strerror(errno));
                close(socket_pair[0]); close(socket_pair[1]); close(internal_pipe[0]); close(internal_pipe[1]);
                kill(pid,SIGKILL);
                exit(ERR_COMM);
            }

            if (read(socket_pair[1], &response_length, 4) < 4)
            {
                printf("[parent] Error at getting response length: %s\n",strerror(errno));
                close(socket_pair[0]); close(socket_pair[1]); close(internal_pipe[0]); close(internal_pipe[1]);
                kill(pid,SIGKILL);
                exit(ERR_COMM);
            }

            if (response_length == EXIT_STATUS) {
                //normal exiting procedure, son already exited
                close(socket_pair[0]);
                close(socket_pair[1]);
                close(internal_pipe[0]);
                close(internal_pipe[1]);
                break;
            }

            //wait for the actual file to be created
            while (access(FIFO_NAME, F_OK) == -1);

            pipe_fd = open(FIFO_NAME, O_RDONLY);

            if (pipe_fd == -1)
            {
                printf("[parent] Error at opening the FIFO : %s",strerror(errno));
                close(socket_pair[0]); close(socket_pair[1]); close(internal_pipe[0]); close(internal_pipe[1]);
                kill(pid,SIGKILL);
                exit(ERR_FILE);
            }

            if (read(pipe_fd, &response, response_length) < response_length)
            {
                printf("[parent] Error at getting response string: %s\n",strerror(errno));
                close(socket_pair[0]); close(socket_pair[1]); close(internal_pipe[0]); close(internal_pipe[1]);
                close(pipe_fd);
                kill(pid,SIGKILL);
                exit(ERR_COMM);
            }
            printf("%s\n", response);
        }
    } else {
        //child process
        close(0);
        int is_logged_in = 0;

        while (1) {
            if (read(socket_pair[0], &command_length, 4) < 4)
            {
                printf("[child] Error when reading command length %s\n",strerror(errno));
                close(socket_pair[0]); close(socket_pair[1]); close(internal_pipe[0]); close(internal_pipe[1]);
                int should_kill = 0;
                response_length = EXIT_STATUS;
                if (write(socket_pair[0], &response_length, 4) < 4) {
                    printf("[child] Error at writing response length : %s\n",strerror(errno));
                    should_kill = 1;
                }
                close(socket_pair[0]);
                close(socket_pair[1]);
                close(internal_pipe[0]);
                close(internal_pipe[1]);
                if (access(FIFO_NAME, F_OK) != -1 && remove(FIFO_NAME) == -1) {
                    printf("Error at removing the FIFO from the filesystem : %s\n", strerror(errno));
                }
                if (should_kill)
                    kill(getppid(),SIGKILL);
                exit(ERR_COMM);
            }
            if (read(internal_pipe[0], command, command_length) < command_length)
            {
                printf("[child] Error when reading the command data %s\n",strerror(errno));
                int should_kill = 0;
                response_length = EXIT_STATUS;
                if (write(socket_pair[0], &response_length, 4) < 4) {
                    printf("[child] Error at writing response length : %s\n",strerror(errno));
                    should_kill = 1;
                }
                close(socket_pair[0]);
                close(socket_pair[1]);
                close(internal_pipe[0]);
                close(internal_pipe[1]);
                if (access(FIFO_NAME, F_OK) != -1 && remove(FIFO_NAME) == -1) {
                    printf("Error at removing the FIFO from the filesystem : %s\n", strerror(errno));
                }
                if (should_kill)
                    kill(getppid(),SIGKILL);
                exit(ERR_COMM);
            }

            char *p = strtok(command, " \t\n");
            if ( p == NULL)
            {
                sprintf(response, "Invalid command, try \"help\" for assistance");
                goto out;
            }
            if (!strcmp(p, LOGIN))
            {
                response[0] = '\0';
                if (is_logged_in == 1) {
                    sprintf(response, "Another user is already logged in");
                    goto out; //discouraged
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

            }
            else if (!strcmp(p, MY_STAT))
            {
                response[0] = '\0';
                if (is_logged_in == 0) {
                    sprintf(response, "You are required to login before executing commands");
                    goto out;
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
            else if (!strcmp(p, MY_FIND))
            {
                response[0] = '\0';
                if (is_logged_in == 0) {
                    sprintf(response, "You are required to login before executing commands");
                    goto out;
                }
                char *cwd = getcwd(NULL, 0); //pointer to current directory
                char *d = strtok(NULL, " \t\n"); //pointer to dirname
                char *f = strtok(NULL, " \t\n"); //pointer to filename
                if (d == NULL)
                {
                    sprintf(response,"Command format: myfind [directory] <filename>");
                    free(cwd);
                    goto out;
                }
                if (f == NULL)
                {
                    //defaults directory to current working directory
                    f = d;
                    d = cwd;
                }
                if (!is_dir(d))
                {
                    sprintf(response,"Error at directory : %s\n",strerror(errno));
                    free(cwd);
                    goto out;
                }
                // directory is fine, start looking for the file;
                find_file(d,f,response);
                if (response[0] == '\0')
                    sprintf(response,"No file with that name found");
                free(cwd);
            }
            else if (!strcmp(p, HELP))
            {
                sprintf(response, "Available commands: \n login <name> \t\t\t command used to"
                        " login with an existing username \n mystat <filepath> \t\t displays "
                        "basic info about the file at filepath\n myfind [directory] <filename>\t"
                        "searches for the file in the directory structure starting at the specified "
                        "directory (defaults to the current working directory)\n"
                        " quit \t\t\t\t command used to exit the program"
                        "\n NOTES: [] means optional argument, <> means mandatory argument");
            } else if (!strcmp(p, QUIT))
            {
                int should_kill = 0;
                response_length = EXIT_STATUS;
                if (write(socket_pair[0], &response_length, 4) < 4) {
                    printf("[child] Error at writing response length : %s\n",strerror(errno));
                    should_kill = 1;
                }
                close(socket_pair[0]);
                close(internal_pipe[0]);
                close(internal_pipe[1]);
                if (access(FIFO_NAME, F_OK) != -1 && remove(FIFO_NAME) == -1) {
                    printf("Error at removing the FIFO from the filesystem : %s\n", strerror(errno));
                }
                if (should_kill)
                    kill(getppid(),SIGKILL);
                exit(0);
            } else
            {
                sprintf(response, "Invalid command, try \"help\" for assistance");
            }

            out:
            response_length = strlen(response) + 1;
            if (write(socket_pair[0], &response_length, 4) < 4 )
            {
                printf("[child] Error at writing the response length : %s\n",strerror(errno));
                close(socket_pair[0]); close(socket_pair[1]); close(internal_pipe[0]); close(internal_pipe[1]);
                kill(getppid(),SIGKILL);
                exit(ERR_COMM);
            }

            if (access(FIFO_NAME, F_OK) == -1 && mkfifo(FIFO_NAME, 0666) == -1)
            {
                printf("[child] Error at creating FIFO in the filesystem: %s\n",strerror(errno));
                close(socket_pair[0]); close(socket_pair[1]); close(internal_pipe[0]); close(internal_pipe[1]);
                kill(getppid(),SIGKILL);
                exit(ERR_FILE);
            }
            pipe_fd = open(FIFO_NAME, O_WRONLY);
            if (pipe_fd == -1) {
                printf("[child] Error at opening FIFO: %s\n",strerror(errno));
            }

            if (write(pipe_fd, response, response_length) < response_length)
            {
                printf("[child] Error at writing the response : %s",strerror(errno));
                kill(getppid(),SIGKILL);
            }
        }
    }
}
