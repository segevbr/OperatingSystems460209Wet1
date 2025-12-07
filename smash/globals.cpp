#include "globals.h"

// example function for printing errors from internal commands
void perrorSmash(const char *cmd, const char *msg)
{
	fprintf(stderr, "smash error: %s%s%s\n",
			cmd ? cmd : "",
			cmd ? ": " : "",
			msg);
}

// Helper function to build original command string from string cmd 
string join_args(const vector<string> args){
	string cmd_string = "";
    for (int i = 0; i < int(args.size()); i++){
        cmd_string += args[i] + " ";
    }
    if (!cmd_string.empty()) cmd_string.pop_back(); // delete last " "
	
    return cmd_string;
}