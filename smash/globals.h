#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstdio>

enum Cmd_err {
	COMMAND_SUCCESSFUL,
	COMMAND_FAILURE
};

void perrorSmash(const char *cmd, const char *msg);
#endif