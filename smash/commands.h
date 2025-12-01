#ifndef COMMANDS_H
#define COMMANDS_H

#include "my_system_call.h"
#include "globals.h"
#include "fd_handler.h"
#include "jobs.h"
#include <string>
#include <vector>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <cerrno>
#include <climits>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#ifndef ARGS_NUM_MAX
#define ARGS_NUM_MAX 80 // Default max arguments if not already defined
#define BUFFER_SIZE 256
#endif


struct cmd {
	char* cmd;
	char* args[ARGS_NUM_MAX];
	
	//vector<string> aliases;

	int numArgs = 0;
	bool isBg = false;
};

// Function Prototypes
void perrorSmash(const char* cmd, const char* msg);

// Built-in commands wrappers
int showpid(const vector<string> &args);
int pwd(const vector<string> &args);
int cd(const vector<string> &args);
int jobs(const vector<string> &args);
int kill(const vector<string> &args);
int fg(const vector<string> &args);
int bg(const vector<string> &args);
int quit(const vector<string> &args);
int diff(const vector<string> &args);


// Built-in commands functions
int showpid_func();
int pwd_func();
int cd_func(const string path);
int jobs_func();
int kill_func(int sig_num, int job_id);
int fg_func(int job_id);
int bg_func(int job_id);
int quit_func(bool kill);
int diff_func(const string path1, const string path2);

// Helper functions
string get_current_wd_string();
void check_fd_err(int err, int sys_call);
int dir_check(const string path);
int close_fd(int &fd);

#endif // COMMANDS_H