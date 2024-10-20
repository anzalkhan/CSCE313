#include <Command.h>
#include <Tokenizer.h>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <array>

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define WHITE   "\033[1;37m"
#define NC      "\033[0m"

/**
 * Return Shell prompt to display before every command as a string
 * 
 * Prompt format is: `{MMM DD hh:mm:ss} {user}:{current_working_directory}$`
 */
void print_prompt() {
    // Use time() to obtain the current system time, and localtime() to parse the raw time data
    time_t now = time(0);
    std::tm *ltm = localtime(&now);
    
    // Parse date time string from raw time data using strftime()
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%b %d %H:%M:%S", ltm);

    // Obtain current user and current working directory by calling getenv() and obtaining the "USER" and "PWD" environment variables
    std::string user = getenv("USER");
    char cwd[500];
    getcwd(cwd, sizeof(cwd));

    // Output prompt
    std::cout << YELLOW << buffer << " " << user << ":" << cwd << "$ " << NC;
}

/**
 * Process shell command line
 * 
 * Each command line from the shell is parsed by '|' pipe symbols, so command line must be iterated to execute each command
 * Example: `ls -l / | grep etc`
 */
void process_commands(Tokenizer &tknr) {
    for (auto cmd : tknr.commands)
        {
                for (auto str : cmd->args)
                {
                        std::cerr << "|" << str << "| ";
                }
                if (cmd->hasInput())
                {
                        std::cerr << "in< " << cmd->in_file << " ";
                }
                if (cmd->hasOutput())
                {
                        std::cerr << "out> " << cmd->out_file << " ";
                }
                std::cerr << std::endl;
        }

        std::vector<std::array<int, 2>> pipes;
        // init pipes
        for (std::vector<Command *>::size_type i = 0; i < tknr.commands.size(); i++)
        {
                std::array<int, 2> pipefd;
                if (pipe(pipefd.data()) < 0)
                {
                        perror("pipe");
                        exit(2);
                }
                pipes.push_back(pipefd);
        }

        

        for (std::vector<Command *>::size_type i = 0; i < tknr.commands.size(); i++) {
                Command cmd = *tknr.commands.at(i);
                // fork to create child
                pid_t pid = fork();
                if (pid < 0)
                { // error check
                        perror("fork");
                        exit(2);
                }

                if (pid == 0)
                { // if child, exec to run command
                        
                        std::vector<char *> args(cmd.args.size() + 1);

                        // Populate the argument vector
                        for (size_t i = 0; i < cmd.args.size(); i++)
                        {
                                args[i] = const_cast<char *>(cmd.args[i].c_str());
                        }
                        args[cmd.args.size()] = nullptr; // Null-terminate the argument list

                        std::cerr << "Running command: " << args[0] << " with args: ";
                        for (size_t i = 0; i < cmd.args.size(); i++)
                        {
                                std::cerr << args[i] << " ";
                        }
                        std::cerr << std::endl;

                        if (i != 0)
                        { // Duplicate previous pipe to opening of new pipe
                                if (dup2(pipes[i - 1][0], STDIN_FILENO) < 0)
                                {
                                        std::cerr << "Error copying pipe to stdin" << std::endl;
                                        perror("dup2");
                                        exit(2);
                                }
                        }
                        if (i != tknr.commands.size() - 1)
                        { // Duplicate pipe to stdout
                                if (dup2(pipes[i][1], STDOUT_FILENO) < 0)
                                {
                                        std::cerr << "Error copying pipe to stdout" << std::endl;
                                        perror("dup2");
                                        exit(2);
                                }
                        }

                        if (cmd.in_file != "")
                        {
                                std::cerr << "Redirecting input from " << cmd.in_file << std::endl;
                                int fd = open(cmd.in_file.c_str(), O_RDONLY);
                                if (fd < 0)
                                {
                                        perror("open");
                                        exit(2);
                                }
                                if (dup2(fd, STDIN_FILENO) < 0)
                                {
                                        perror("dup2");
                                        exit(2);
                                }
                                close(fd);
                        }
                        if (cmd.out_file != "")
                        {
                                std::cerr << "Redirecting output to " << cmd.out_file << std::endl;
                                int fd = open(cmd.out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                                if (fd < 0)
                                {
                                        perror("open");
                                        exit(2);
                                }
                                if (dup2(fd, STDOUT_FILENO) < 0)
                                {
                                        perror("dup2");
                                        exit(2);
                                }
                                close(fd);
                        }

                        if (execvp(args[0], args.data()) < 0)
                        { // error check
                                perror("execvp");
                                exit(2);
                        }
                }
                else
                { // if parent, wait for child to finish
                        int status = 0;
                        waitpid(pid, &status, 0);
                        std::cerr << "Command " << cmd.args[0] << " exited with status " << status << std::endl;
                        close(pipes[i][1]);

                        if (status > 1)
                        { // exit if child didn't exec properly
                                exit(status);
                        }
                }
        }
}

int main() {
    char* previous_dir = nullptr;
    // Use setenv() and getenv() to set an environment variable for the previous PWD from 'PWD'
            while(1) {
                
                print_prompt();

                // get user inputted command
                std::string input;
                getline(std::cin, input);

                if (input == "exit")
                {
                        // print exit message and break out of infinite loop
                        std::cout << RED << "Now exiting shell..." << std::endl
                                  << "Goodbye" << NC << std::endl;
                        break;
                }

                // get tokenized commands from user input
                Tokenizer tknr(input);
                if (tknr.hasError())
                {
                        // continue to next prompt if input had an error
                        continue;
                }
                // // print out every command token-by-token on individual lines
                // // prints to cerr to avoid influencing autograder

                bool isBackground = false;

                for(auto cmd : tknr.commands)
                {
                        if(cmd->isBackground())
                        {
                                isBackground = true;
                        }
                }

                if(tknr.commands[0]->args[0] == "cd")
                {
                        if(tknr.commands[0]->args[1] == "-")
                        {
                                if(previous_dir == nullptr)
                                {
                                        std::cerr << "No previous directory" << std::endl;
                                        continue;
                                }

                                if(chdir(previous_dir) < 0)
                                {
                                        perror("chdir");
                                }
                                continue;
                        }
                        if(previous_dir != nullptr)
                        {
                                free(previous_dir);
                        }
                        previous_dir = get_current_dir_name();
                        if(chdir(tknr.commands[0]->args[1].c_str()) < 0)
                        {
                                perror("chdir");
                        }
                        continue;
                }

                if(isBackground)
                {
                        std::cerr << "Running in background" << std::endl;
                        pid_t pid = fork();
                        if(pid < 0)
                        {
                                perror("fork");
                                exit(2);
                        }
                        if(pid == 0)
                        {
                                process_commands(tknr);
                                exit(0);
                        }
                }
                else {
                        process_commands(tknr);
                }
                
        }
        if(previous_dir != nullptr)
                {
                        free(previous_dir);
                }
}
