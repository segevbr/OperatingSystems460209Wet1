#include "globals.h"

// example function for printing errors from internal commands
void perrorSmash(const char *cmd, const char *msg)
{
	fprintf(stderr, "smash error: %s%s%s\n",
			cmd ? cmd : "",
			cmd ? ": " : "",
			msg);
}