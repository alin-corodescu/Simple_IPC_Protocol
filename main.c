//
// Created by alin on 10/30/16.
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <bits/socket_type.h>
#include <bits/socket.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>

#define COMM_LENGTH 500
#define RESPONSE_LENGTH 1000
#define FIFO_NAME ".hidden_fifo"
#define EXIT_STATUS -1
void start_handling_commands();



int main()
{
    start_handling_commands();
    return 0;
}

void start_handling_commands()
{
    char command[COMM_LENGTH];
    char response[RESPONSE_LENGTH];
    int socket_pair[2],internal_pipe[2],pipe_fd;
    int pid;
    int command_length, response_length;

    //create the internal pipe for commands passing
    pipe(internal_pipe);

    //create socket pair for message length communication
    socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair);


    while (true)
    {
        pid = fork();
        if (!pid)
        {
            //parent process
            printf("SIPCPP\\:> ");
            fgets(command, COMM_LENGTH, stdin);
            command_length = strlen(command);

            write(socket_pair[1],&command_length,4);
            write(internal_pipe[1],command,command_length + 1);

            read(socket_pair[1],&response_length,4);

            if (response_length == EXIT_STATUS)
                break;

            //wait for the actual file to be created
            while (access( FIFO_NAME, F_OK ) == -1 );
            pipe_fd = open(FIFO_NAME, O_RDONLY);

            read(pipe_fd,&response,response_length);

            printf("%s\n",response);
        }
        else
        {
            //child process
            read(socket_pair[0],&command_length,4);
            read(internal_pipe[0],command,command_length);

            /* handle the command here */


            response_length = strlen(response);
            write(socket_pair[0],&response_length,4);

            mkfifo(FIFO_NAME, 0666);
            pipe_fd = open(FIFO_NAME, O_WRONLY);
            write(pipe_fd, response,response_length + 1);
        }


    }


}
