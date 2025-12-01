#include "fd_handler.h"

// Helper function - check if path is directory (doens't handle prints)
// returns Fd_res enum value in case all sys calls successful
// otherwise returns Cmd_err enum
int path_check(const string path){
	int fd_dir_test = (int)my_system_call(SYS_OPEN, path.c_str(), 
											O_RDONLY | O_DIRECTORY);
	int err = errno;
	if (fd_dir_test >= 0) { // path is directory
		// try to close fd_dir_test
		if (close_fd(fd_dir_test) == COMMAND_FAILURE){
            return COMMAND_FAILURE;
        }
		return IS_DIR;
	}
    if (err == ENOENT){
        return PATH_NOT_EXIST;
    }
    
    // check if path is a file
    if (err == ENOTDIR){
        return IS_FILE;
    }

    return COMMAND_FAILURE; // some other sys call error
}

// Helper function - close file descriptor 
int close_fd(int &fd){
	int err;
	if (my_system_call(SYS_CLOSE, fd) == -1){ // try to close file descriptor
		err = errno;
		check_fd_err(err, SYS_CLOSE);
		return COMMAND_FAILURE; // sys call failed
	}
    return COMMAND_SUCCESSFUL;
}

// Helper function for printing error of open() and close() system calls 
void check_fd_err(int err, int sys_call){
	if (err == ENONET) {
		perrorSmash("diff", "expected valid paths for files");
	} else if (err == EISDIR) {
		perrorSmash("diff", "paths are not files");
	} else {
		if (SYS_OPEN) perrorSmash("open", "failed");
		else if (SYS_CLOSE) perrorSmash("close", "failed");
		else if (SYS_READ) perrorSmash("read", "failed");
	}
}