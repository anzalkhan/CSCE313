/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <stdio.h>
#include <stdlib.h>
using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    // TODO: add functionality
    // Create pipe
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create child to run first command
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // In child, redirect output to write end of pipe
    // Close the read end of the pipe on the child side.
    // In child, execute the command
    if (pid1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        // close(fd[1]);
        execvp(cmd1[0], cmd1); // Execute the first command
        perror("execvp"); // If execvp fails
        exit(EXIT_FAILURE);
    }


    // Create another child to run second command
    // In child, redirect input to the read end of the pipe
    // Close the write end of the pipe on the child side.
    // Execute the second command.
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        // close(fd[0]); 

        execvp(cmd2[0], cmd2); 
        perror("execvp"); 
        exit(EXIT_FAILURE);
    }

    // Reset the input and output file descriptors of the parent.
    close(fd[1]);
    close(fd[0]);

    return 0;
}
