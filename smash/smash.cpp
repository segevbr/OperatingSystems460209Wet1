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
#include <algorithm>
#include <unordered_map>
#include "jobs.h"
#include "globals.h"

#define CMD_LENGTH_MAX 80
/*=============================================================================
* classes/structs declarations
=============================================================================*/
Jobs_list jobs_list;
pid_t fg_process;
pid_t smash_pid = getpid();
string fg_command_str = "";

int alias(const vector<string> &args);

int unalias(const vector<string> &args);

/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];

typedef int (*CmdHandler)(const vector<string> &args);

unordered_map<string, CmdHandler> commandTable = {
        {"showpid", showpid},
        {"pwd",     pwd},
        {"cd",      cd},
        {"jobs",    jobs},
        {"kill",    kill},
        {"fg",      fg},
        {"bg",      bg},
        {"quit",    quit},
        {"diff",    diff},
        {"alias",   alias},
        {"unalias", unalias}
};

unordered_map<string, char *> aliasTable;

int bigParser(char *line);

int smallParser(string cmd_stg);

int run_command(vector<string> &commands);

int run_external_command(vector<string> &commands, bool is_bg);

void add_string_to_vector(vector<string> &commands, char *start, char *end);

int perform_right_command(vector<string> &command, bool is_bg);

void split_by_equal(const string &s, string &left, string &right);

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char *argv[]) {
    fg_process = smash_pid; // set smash as foreground process at the start

    // SIGINT (CTRL+C)
    struct sigaction sa_int;
    sa_int.sa_handler = &catch_ctrl_c;
    sa_int.sa_flags = 0;
    sigfillset(&sa_int.sa_mask);
    sigaction(SIGINT, &sa_int, NULL);

    // SIGTSTP (CTRL+Z)
    struct sigaction sa_stp;
    sa_stp.sa_handler = &catch_ctrl_z;
    sa_stp.sa_flags = 0;
    sigfillset(&sa_stp.sa_mask);
    sigaction(SIGTSTP, &sa_stp, NULL);


    char _cmd[CMD_LENGTH_MAX];
    while (1) {
        printf("smash > ");
        if (fgets(_line, CMD_LENGTH_MAX, stdin) == NULL) {
            if (ferror(stdin) && errno == EINTR) {
                clearerr(stdin); // Clear the error so we can read again
                continue; // Go back to start of while(1) to reprint "smash > "
            }
            // Check for EOF or Error
            break;
        }
        strcpy(_cmd, _line);
        //execute command
        int res = bigParser(_cmd);
        if (res == COMMAND_FAILURE) {
            continue;
        }
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

    bool last_command_success = COMMAND_SUCCESSFUL;

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
        if (last_command_success == COMMAND_SUCCESSFUL) {
            res = smallParser(commands[i]);
            last_command_success = res;
        } else {
            return COMMAND_FAILURE;
        }
    }
    return res;
}

void add_string_to_vector(vector<string> &commands, char *start, char *end) {
    char *cmd = strndup(start, end - start);
    string str_cmd = cmd;
    commands.push_back(str_cmd);

    free(cmd);
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

    int result = run_command(args);

    free(mod_str);

    return result;
}

int run_command(vector<string> &command) {
    jobs_list.garbage_collector();
    bool isBg = command.back() == "&";
    if (isBg) {
        command.pop_back();
    }
    int success = -1;

    //find the right command
    string command_name = command[0];
    auto table_command = commandTable.find(command_name);
    if (table_command == commandTable.end()) { //either command is alias or external

        auto alias_command = aliasTable.find(command_name);
        if (alias_command == aliasTable.end()) {// command doesn't exist - external
            success = run_external_command(command, isBg);
        } else { //command is alias
            char *command_of_alias = alias_command->second;
            return bigParser(command_of_alias);
        }

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
            jobs_list.add_job(command, pid);
        }

    } else {
        fg_command_str = join_args(command);
        success = perform_right_command(command, isBg);
    }
    return success;
}

int perform_right_command(vector<string> &command, bool is_bg) {
    string command_name = command[0];
    auto table_command = commandTable.find(command_name);
    CmdHandler handler = table_command->second;

    return handler(command);
}

int run_external_command(vector<string> &command, bool is_bg) {
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
        setpgrp();
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
            // External command is now the fg command
            fg_process = pid;
            fg_command_str = join_args(command);

            // Smash waits for command to finish
            int status;
            pid_t wait_res = my_system_call(SYS_WAITPID, pid, &status, WUNTRACED);

            if (wait_res == -1) {
                if (errno == EINTR){
                    errno = 0; // clear errno
                }
                else perrorSmash("waitpid", "failed");
                return COMMAND_FAILURE;
            }

            if(WIFEXITED(status)){
                int exit_status = WEXITSTATUS(status);
                success = (exit_status == 0) ? COMMAND_SUCCESSFUL : COMMAND_FAILURE;
            } else {
                success = COMMAND_FAILURE;
            }

            // Sets fg_process as himself again
            fg_process = smash_pid;
        }
        // command supposed to run in background so add it to list
        if (is_bg) jobs_list.add_job(command, pid);
    }

    return success;
}

int alias(const vector<string> &args) {
    //validations
    if (args.size() < 2 || args[1].find('=') == string::npos) {
        perrorSmash("alias", "invalid arguments");
        return COMMAND_FAILURE;
    }

    //making it a full string again
    string command_full;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) {
            command_full += " ";
        }
        command_full += args[i];
    }

    //dividing the alias command and action
    string command_name_to_alias;
    string command_string_to_alias;
    split_by_equal(command_full, command_name_to_alias, command_string_to_alias);
    if (command_string_to_alias.front() != '"' || command_string_to_alias.back() != '"') {
        perrorSmash("alias", "invalid arguments");
        return COMMAND_FAILURE;
    }
    command_string_to_alias = command_string_to_alias.substr(1, command_string_to_alias.size() - 2); //delete ""

    char *command_char_kochavit_to_alias = strdup(command_string_to_alias.c_str());

    auto alias_command = aliasTable.find(command_name_to_alias);
    if (alias_command == aliasTable.end()) {// if command already alias
        aliasTable[command_name_to_alias] = command_char_kochavit_to_alias;
    } else {
        alias_command->second = command_char_kochavit_to_alias;
    }

    return COMMAND_SUCCESSFUL;
}

int unalias(const vector<string> &args) {

    if (args.size() != 2) {
        perrorSmash("unalias", "invalid arguments");
        return COMMAND_FAILURE;
    }

    auto alias_command = aliasTable.find(args[1]);
    if (alias_command == aliasTable.end()) { //no alias to delete
        perrorSmash("unalias", "invalid arguments");
        return COMMAND_FAILURE;
    }

    free(alias_command->second);
    aliasTable.erase(alias_command);
    return COMMAND_SUCCESSFUL;

}

void split_by_equal(const string &s, string &left, string &right) {
    size_t pos = s.find('=');

    left = s.substr(0, pos);
    right = s.substr(pos + 1);
}