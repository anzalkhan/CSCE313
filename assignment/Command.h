#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <string>
#include <vector>

/*
 * class that stores information about a command
 *
 * in_file  - string containing the redirected input filename, if it exists
 * out_file - string containing the redirected output filename, if it exists
 * args     - vector of strings containing the arguments of the command
 *
 * Whether or not the command should be run in the background is also stored
 */
class Command
{
        private:
                std::string              cmd;           // full command stored for internal convenience
                std::vector<std::string> inner_strings; // list of quoted strings in command
                bool                     bg;            // whether or not the command should be run in the background

        public:
                std::string              in_file;  // filename of redirected input file, if it exists
                std::string              out_file; // filename of redirected output file, if it exists
                std::vector<std::string> args;     // command arguments

                // constructor - takes command and calls internal convenience
                //               functions to parse the arguments
                Command(const std::string _cmd, std::vector<std::string> _inner_strings);
                ~Command() {}

                bool hasInput();
                bool hasOutput();
                bool isBackground();

        private:
                // convenience functions to trim whitespace, find input/output filename,
                // and parse arguments
                std::string trim(const std::string in);
                void        findInOut();
                void        parseArgs();
};

#endif
