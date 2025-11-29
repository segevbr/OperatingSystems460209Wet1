#include "commands.h"

// ------- Mock -------- // 


// ------- Built-in commands wrappers ------ //
int showpid(const vector<string> &args) {
	if (args.size() != 1) {
		perrorSmash("showpid", "expected 0 arguments");
		return COMMAND_FAILURE;
	}
	return showpid_func();
}
int pwd(const vector<string> &args){
	if (args.size() != 1) {
		perrorSmash("pwd", "expected 0 arguments");
		return COMMAND_FAILURE;
	}
	return pwd_func();
}
int cd(const vector<string> &args) {
	if (args.empty()) return COMMAND_FAILURE; 
	if (args.size() != 2 || args[1].empty()) {
		perrorSmash("cd", "expected 1 argument");
		return COMMAND_FAILURE;
	} 
	string path = args[1];
	return cd_func(path);
}
// int jobs(const vector<string> &args);
// int kill(const vector<string> &args);
// int fg(const vector<string> &args);
// int bg(const vector<string> &args);
// int quit(const vector<string> &args);
int diff(const vector<string> &args){
	if (args.empty()) return COMMAND_FAILURE;
	if (args.size() != 3 || args[1].empty() || args[2].empty()){
		perrorSmash("diff", "expected 2 arguments");
		return COMMAND_FAILURE;
	}
	string path1 = args[1];
	string path2 = args[2];
	return diff_func(path1, path2);
}
// ----------------------------------------- //

// ------------- Built-in commands ----------- //
// Show the PID of smash proccess
// getppid() never throws any errors therefore is always successful
int showpid_func() {
	cout << "smash pid is " << getppid() << endl;
	return COMMAND_SUCCESSFUL;
}

// Print current working directory of the process running it 
int pwd_func() {
	string cwd = get_current_wd_string(); 

    if (!cwd.empty()){
        string path(cwd); 
        cout << path << endl;
		return COMMAND_SUCCESSFUL;
    } 
	return COMMAND_FAILURE;	
}

// Helper function to get current working directory string
string get_current_wd_string() {
    char buffer[PATH_MAX];

	if (getcwd(buffer, PATH_MAX) != NULL) {
        return string(buffer);
    } else {
		perrorSmash("getcwd", "failed");
		return ""; 
	}
}

// previous working directory storage
static string prev_wd = "";

// Assumes path string isn't empty
int cd_func(const string path) {
	string curr_wd = get_current_wd_string();
	if (path.compare("-") == 0){
		// if no previous directory - return error
		if (prev_wd.empty()) {
			perrorSmash("cd", "old pwd not set");
			return COMMAND_FAILURE;
		}

		// go back to previous working directory
		if (chdir(prev_wd.c_str()) != 0) {
			perrorSmash("chmod", "failed");
			return COMMAND_FAILURE;
		}
		
		cout << prev_wd << endl; // print previous working directory
		prev_wd = curr_wd; // update previous working directory
		
	} else {
		// chdir handles both .. and "path" of some sort.
		if (chdir(path.c_str()) != 0){
			int err = errno;
			// Error checking
			// ENOENT - No such file or directory 
			// ENOTDIR - Not a directory
			if (err == ENOENT) { 
				perrorSmash("cd", "target directory does not exist");
			} else if (err == ENOTDIR) { 
				string err_msg = path + ": not a directory";
				perrorSmash("cd", err_msg.c_str());
			}
			return COMMAND_FAILURE;
		}
		prev_wd = curr_wd; // update previous working directory
	}
	return COMMAND_SUCCESSFUL;
}

int jobs_func() {
	return COMMAND_SUCCESSFUL;
}

int kill_func(int sig_num, int job_id){
	return COMMAND_SUCCESSFUL;
}

int diff_func(const string path1, const string path2) {
	int fd1 = -1;
	int fd2 = -1;
	char buf1[BUFFER_SIZE]; 
	char buf2[BUFFER_SIZE];
	ssize_t bytes_read1; // read() returns ssize_t
	ssize_t bytes_read2;
	int result = COMMAND_FAILURE; // assume files are different

	// Directory check for both paths
	int res1 = path_check(path1);
	int res2 = path_check(path2);

	if (res1 == IS_DIR || res2 == IS_DIR){ 
		perrorSmash("diff", "paths are not files");
		return COMMAND_FAILURE;
	} else if (res1 == PATH_NOT_EXIST || res2 == PATH_NOT_EXIST){
		perrorSmash("diff", "expected valid paths for files");
		return COMMAND_FAILURE;
	}

	// open file 1
	fd1 = (int)my_system_call(SYS_OPEN, path1.c_str(), O_RDONLY);
	int err = errno;
	if (fd1 == -1){
		check_fd_err(err, SYS_OPEN);
		return COMMAND_FAILURE;
	}
	
	// open file 2
	fd2 =(int)my_system_call(SYS_OPEN, path2.c_str(), O_RDONLY);
	err = errno;
	if (fd2 == -1){
		check_fd_err(err, SYS_OPEN); // failed to open file2 
		int close_res = close_fd(fd1); // close file1
		if (close_res == COMMAND_FAILURE) return COMMAND_FAILURE;
	}
	
	// read both files and compare
	while (true) {
		// Read chunk from file 1
		bytes_read1 = my_system_call(SYS_READ, fd1, buf1, BUFFER_SIZE);
		err = errno;
		if (bytes_read1 == -1){
			check_fd_err(err, SYS_READ);
			break;
		}

		// Read chunk from file 2
		bytes_read2 = my_system_call(SYS_READ, fd2, buf2, BUFFER_SIZE);
		err = errno;
		if (bytes_read2 == -1){
			check_fd_err(err, SYS_READ);
			break;
		}

		// Check if both reads are EOF in the same time 
		if (bytes_read1 == 0 && bytes_read2 == 0){
			result = COMMAND_SUCCESSFUL;
			break;
		}

		// Check if the number of bytes differ 
		if (bytes_read1 != bytes_read2){
			result = COMMAND_FAILURE;
			break;
		}

		// Compare the content of the two buffers
		if (memcmp(buf1, buf2, bytes_read1) != 0){
			result = COMMAND_FAILURE;
			break;
		}
		// continue to check on next chunks
	}

	// Try to close both files, if we can't close one of them result = command failed
	if (close_fd(fd1) == COMMAND_FAILURE || close_fd(fd2) == COMMAND_FAILURE)
		result = COMMAND_FAILURE;
	
	// print the result
	cout << result << endl;
	return result;
}

