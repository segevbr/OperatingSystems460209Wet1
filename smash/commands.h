#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>

using namespace std;
// Assuming ARGS_NUM_MAX is needed and defined here or globally.
#ifndef ARGS_NUM_MAX
#define ARGS_NUM_MAX 80 // Default max arguments if not already defined
#endif

// In C++, 'typedef struct name {} name;' is redundant.
// We just define the struct.
struct cmd {
	char* cmd;
	char* args[ARGS_NUM_MAX];
	
	//vector<string> aliases;

	int numArgs = 0;
	bool isBg = false;
};

// Function Prototypes
void perrorSmash(const char* cmd, const char* msg);
int bigParser(char* line);
int smallParser(string cmd_stg);
void add_string_to_vector(vector<string> &commands, char *start, char *end);
#endif // COMMANDS_H