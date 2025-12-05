#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstdio>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <cstring>

using namespace std;

#ifndef WHOHANG
#define WHOHANG 1
#endif

#ifndef WUNTRACED
#define WUNTRACED 2
#endif 

extern pid_t fg_process;
extern pid_t smash_pid;

enum Cmd_err {
	COMMAND_SUCCESSFUL,
	COMMAND_FAILURE
};

void perrorSmash(const char *cmd, const char *msg);
string join_args(const vector<string> args);
#endif