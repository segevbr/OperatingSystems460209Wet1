#ifndef FD_HANDLER_H
#define FD_HANDLER_H

#include "my_system_call.h"
#include "globals.h"
#include <string>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <cerrno>
#include <climits>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

// enum for fd checks (readability)
enum Fd_res {
	IS_DIR = 2, // to avoid confilts with Cmd_err enum
	IS_FILE,
    PATH_NOT_EXIST
};

int close_fd(int &fd);
int path_check(const string path);
void check_fd_err(int err, int sys_call);
#endif // FD_HANDLER_H