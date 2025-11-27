#include "commands.h"

// Required for C++ standard library features
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>

using namespace std;

// example function for printing errors from internal commands
void perrorSmash(const char *cmd, const char *msg)
{
	fprintf(stderr, "smash error:%s%s%s\n",
			cmd ? cmd : "",
			cmd ? ": " : "",
			msg);
}

