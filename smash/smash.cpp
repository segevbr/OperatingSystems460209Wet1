//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include "commands.h"
#include "signals.h"
#include "my_system_call.h"
#include <cstring>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <unistd.h>
#include <unordered_map>


#define SUCCESS 1
#define FAIL 0

#define CMD_LENGTH_MAX 80
/*=============================================================================
* classes/structs declarations
=============================================================================*/

/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];

typedef int (*CmdHandler)(const vector<string> &args);

unordered_map<string, CmdHandler> commandTable = { //todo change for the right commands when we have their names
        {"cd",   cmdCd},
        {"kill", cmdKill},
        {"pwd",  cmdPwd}
};

int bigParser(char *line);

int smallParser(string cmd_stg);

int run_command(vector<string> &commands);

void run_external_command(vector<string> &commands, bool is_bg);

void add_string_to_vector(vector<string> &commands, char *start, char *end);

int perform_right_command(vector<string> &command, bool is_bg);


/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char *argv[]) {
    char _cmd[CMD_LENGTH_MAX];
    while (1) {
        printf("smash > ");
        fgets(_line, CMD_LENGTH_MAX, stdin);
        strcpy(_cmd, _line);
        //execute command
        bigParser(_cmd);
        //initialize buffers for next command
        _line[0] = '\0';
        _cmd[0] = '\0';
    }

    return 0;
}

int bigParser(char *line) {

    // Create pointer token for strtok operation
    char *ptr = line;
    char *start = line;

    vector<string> commands;

    while (*ptr) {
        if (!strncmp(ptr, "&&", 2)) { // Success == 0
            add_string_to_vector(commands, start, ptr);
            ptr += 2;
            start = ptr;
        } else {
            ptr++;
        }
    }
    add_string_to_vector(commands, start, ptr);

    int res;
    for (size_t i = 0; i < commands.size(); i++) {
        res = smallParser(commands[i]);
    }
    return res;
}

void add_string_to_vector(vector<string> &commands, char *start, char *end) {
    char *cmd = strndup(start, end - start);
    string str_cmd = cmd;
    commands.push_back(str_cmd);
}

int smallParser(string cmd_stg) {
    char *mod_str = strdup(cmd_stg.c_str());

    const char *delims = " \t\n";           // parsing should be done by spaces, tabs or newlines
    char *token = strtok(mod_str, delims); // read strtok documentation - parses string by delimiters

    if (!token) {
        free(mod_str);
        return -1;
    }
    string cmd = token;

    vector<string> args;
    args.push_back(cmd);

    for (int i = 1; i < ARGS_NUM_MAX; i++) {
        token = strtok(NULL, delims);
        if (token == NULL) {
            break;
        }

        string arg = token;
        args.push_back(arg);
    }

    run_command(args);

    free(mod_str);

    return 0;
}

int run_command(vector<string> &command) {
    bool isBg = command.back() == "&";
    isBg ? cout << "im" << endl : cout << "not" << endl;
    int success = -1;

    //find the right command
    string command_name = command[0];
    auto table_command = commandTable.find(command_name);
    if (table_command == commandTable.end()) { // command doesn't exist - external
        run_external_command(command, isBg);
    } else if (isBg) {
        pid_t pid = (pid_t) my_system_call(SYS_FORK);
        if (pid < 0) { //if failed
            perrorSmash("fork", "failed");
            exit(1);
        }
        if (pid == 0) { //child process
            setpgrp();
            perform_right_command(command, isBg);
        }
        if (pid > 0) {
            //todo add to jobs
        }

    } else {
        perform_right_command(command, isBg);
    }
    return success;
}

int perform_right_command(vector<string> &command, bool is_bg) {
    string command_name = command[0];
    auto table_command = commandTable.find(command_name);
    CmdHandler handler = table_command->second;
    return handler(command);
}

void run_external_command(vector<string> &command, bool is_bg) {
    //convert  to char*
    vector<char *> execArgs;
    for (int i = 0; i < (int) command.size(); i++) {
        execArgs.push_back(const_cast<char *>(command[i].c_str()));
    }
    execArgs.push_back(nullptr);

    //fork
    int success = -1;
    pid_t pid = (pid_t) my_system_call(SYS_FORK);
    if (pid < 0) { //if failed
        perrorSmash("fork", "failed");
        exit(1);
    }
    if (pid == 0) { //child process
        my_system_call(SYS_EXECVP, execArgs[0], execArgs.data());
        //if reached here - failed exec
        if (errno == ENOENT) {
            perrorSmash("external", "cannot find program");
        } else {
            perrorSmash("external", "invalid command");
        }
        exit(1);
    }
    if (pid > 0) { //parent process
        if (!is_bg) {
            my_system_call(SYS_WAITPID, pid, &success, 0);
        }
    }
}



