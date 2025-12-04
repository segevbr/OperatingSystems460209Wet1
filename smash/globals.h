	#ifndef GLOBALS_H
	#define GLOBALS_H

	#include <cstdio>
	#include <unistd.h>

	extern pid_t fg_process;
	extern pid_t smash_pid;

	enum Cmd_err {
		COMMAND_SUCCESSFUL,
		COMMAND_FAILURE
	};

	void perrorSmash(const char *cmd, const char *msg);
	#endif