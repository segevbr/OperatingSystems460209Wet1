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
	fprintf(stderr, "smash error: %s%s%s\n",
			cmd ? cmd : "",
			cmd ? ": " : "",
			msg);
}

/*
 * just mock actions!!!
 */

int cmdCd(const vector <string> &args) {
    cout << "this is the cd action" << endl;
    return 1;
}

int cmdPwd(const vector <string> &args) {
    cout << "this is the pwd action" << endl;
    return 1;
}

int cmdKill(const vector <string> &args) {
    cout << "this is the kill action" << endl;
    return 1;
}


/*
 * just mock actions!!!
 */

