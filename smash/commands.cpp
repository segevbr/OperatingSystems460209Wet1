//commands.c
#include "commands.h"

using namespace std;

typedef struct cmd {
	char* cmd;
	char* args[ARGS_NUM_MAX];
	
	//vector<string> aliases;

	int numArgs;
	bool isBg;
} cmd;

// example function for printing errors from internal commands
void perrorSmash(const char* cmd, const char* msg)
{
    fprintf(stderr, "smash error:%s%s%s\n",
        cmd ? cmd : "",
        cmd ? ": " : "",
        msg);
}

int bigParser(char* line){
	char* delims = "&&";
	// Create pointer token for strtok operation
	char* token = strtok(line, delims);
	vector<string> commands;

	if (!line) return INVALID_COMMAND;
	else {
		while (token != NULL){
			string cmd_stg = token;
			
			commands.push_back(cmd_stg);

			token = strtok(NULL, delims);
		}
	}
	for (int i = 0; i < commands.size(); i++){
		int res;
		res = smallParser(commands[i]);
	}
	return 0;
}

// example function for parsing commands
int smallParser(string cmd_stg)
{
	char* delimiters = " \t\n"; //parsing should be done by spaces, tabs or newlines
	char* cmd = strtok(cmd_stg, delimiters); //read strtok documentation - parses string by delimiters

	if(!cmd)
		return INVALID_COMMAND; // this means no tokens were found, most like since command is invalid
	
	char* args[ARGS_NUM_MAX];
	int nargs = 0;
	args[0] = cmd; //first token before spaces/tabs/newlines should be command name
	for(int i = 1; i < ARGS_NUM_MAX; i++)
	{
		args[i] = strtok(NULL, delimiters); //first arg NULL -> keep tokenizing from previous call
		if(!args[i])
			break;
		nargs++;
	}

	return 0;
}
