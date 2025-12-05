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
Jobs_list jobs_list;
pid_t fg_process;
pid_t smash_pid = getpid();
string fg_command_str = "";
/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];

typedef int (*CmdHandler)(const vector<string> &args);

unordered_map<string, CmdHandler> commandTable = { //todo change for the right commands when we have their names
        {"showpid", showpid},
        {"pwd",     pwd},
        {"cd",      cd},
        {"jobs",    jobs},
        {"kill",    kill},
        {"fg",      fg},
        {"bg",      bg},
        {"quit",    quit},
        {"diff",    diff},

};

int bigParser(char *line);

int smallParser(string cmd_stg);

int run_command(vector<string> &commands);

void run_external_command(vector<string> &commands, bool is_bg);

void add_string_to_vector(vector<string> &commands, char *start, char *end);

int perform_right_command(vector<string> &command, bool is_bg);

void garbage_collector();

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
    
    // SIGCHLD (Background job finished)
    struct sigaction sa_chld;
    sa_chld.sa_handler = &catch_sigchld;
    sa_chld.sa_flags = SA_RESTART; 
    sigfillset(&sa_chld.sa_mask);
    sigaction(SIGCHLD, &sa_chld, NULL);
    

    char _cmd[CMD_LENGTH_MAX];
    while (1) {
        // garbage_collector();
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
    if (isBg) {
        command.pop_back();
    }
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
            jobs_list.add_job(command, pid);
        }

    } else {
        fg_command_str = join_args(command);
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
            // External command is now the fg command
            fg_process = pid;
            fg_command_str = join_args(command);
            // Smash waits for command to finish
            pid_t wait_res = my_system_call(SYS_WAITPID, pid, &success, WUNTRACED);

            if (wait_res == -1){
                perrorSmash("waitpid", "failed");
            } 
            // Sets fg_process as himself again
            fg_process = smash_pid;
        }
        // command supposed to run in background so add it to list
        if (is_bg) jobs_list.add_job(command, pid); 
    }
}

// Clears job in case they are finished
void garbage_collector() {
    int status;
    for (auto it = jobs_list.jobs_list.begin(); it != jobs_list.jobs_list.end();){
        if (it->second.job_state == BG) {
            pid_t pid = it->second.job_pid;
            pid_t code = my_system_call(SYS_WAITPID, pid, &status, WHOHANG);

            if (code == -1){
                perrorSmash("waitpid", "failed");
                ++it;
            } else if (code > 0){
                it = jobs_list.jobs_list.erase(it);
                continue;
            }
        }
        ++it;
    }
}

