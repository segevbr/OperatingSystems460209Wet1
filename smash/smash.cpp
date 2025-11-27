//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include "commands.h"
#include "signals.h"
#include <cstring>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>

#define CMD_LENGTH_MAX 80
/*=============================================================================
* classes/structs declarations
=============================================================================*/

/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];

int bigParser(char* line);
int smallParser(string cmd_stg);
void add_string_to_vector(vector<string> &commands, char *start, char *end);

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	char _cmd[CMD_LENGTH_MAX];
	while(1) {
		printf("smash > ");
		fgets(_line, CMD_LENGTH_MAX, stdin);
		strcpy(_cmd, _line);
		//execute command
		bigParser(_cmd);
		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
	}

	return 0;
}

int bigParser(char *line)
{
	if (!line)
		return -1;

	// Create pointer token for strtok operation
	char *ptr = line;
	char *start = line;

	vector<string> commands;

	while (*ptr)
	{
		if (!strncmp(ptr, "&&", 2))
		{ // Success == 0
			add_string_to_vector(commands, start, ptr);
			ptr += 2;
			start = ptr;
		}
		else
		{
			ptr++;
		}
	}
	add_string_to_vector(commands, start, ptr);

	int res;
	for (size_t i = 0; i < commands.size(); i++)
	{
		res = smallParser(commands[i]);
	}
	return res;
}

void add_string_to_vector(vector<string> &commands, char *start, char *end)
{
	char *cmd = strndup(start, end - start);
	string str_cmd = cmd;
	commands.push_back(str_cmd);
}

int smallParser(string cmd_stg)
{
	char *mod_str = strdup(cmd_stg.c_str());
	if (!mod_str)
		return -1;

	const char *delims = " \t\n";		   // parsing should be done by spaces, tabs or newlines
	char *token = strtok(mod_str, delims); // read strtok documentation - parses string by delimiters

	if (!token)
	{
		free(mod_str);
		return -1;
	}
	string cmd = token;

	vector<string> args;
	args.push_back(cmd);

	for (int i = 1; i < ARGS_NUM_MAX; i++)
	{
		token = strtok(NULL, delims);
		if (token == NULL)
			break;

		string arg = token;
		args.push_back(arg);
	}
	
	free(mod_str);

	return 0;
}
