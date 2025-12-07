#include "commands.h"

extern Jobs_list jobs_list;
extern string fg_command_str;

// ------- Built-in commands wrappers ------ //
int showpid(const vector<string> &args) {
    if (args.size() != 1) {
        perrorSmash("showpid", "expected 0 arguments");
        return COMMAND_FAILURE;
    }
    return showpid_func();
}

int pwd(const vector<string> &args) {
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

int jobs(const vector<string> &args) {
    if (args.empty()) return COMMAND_FAILURE;
    if (args.size() != 1) {
        perrorSmash("jobs", "expected 0 arguments");
        return COMMAND_FAILURE;
    }
    return jobs_func();
}

int kill(const vector<string> &args) {
    if (args.empty()) return COMMAND_FAILURE;
    else if (args.size() != 3 || !is_number(args[1]) || !is_number(args[2])) {
        perrorSmash("kill", "invalid_arguments");
        return COMMAND_FAILURE;
    }
    int signum = stoi(args[1]);
    if (signum < 0 || signum > 20) {
        perrorSmash("kill", "invalid arguments");
        return COMMAND_FAILURE;
    }

    int job_id = stoi(args[2]);
    return kill_func(signum, job_id);
}

int fg(const vector<string> &args) {
    if (args.empty()) return COMMAND_FAILURE;
    if (args.size() == 1) {
        return fg_func(0);
    } else if (args.size() == 2 && is_number(args[1])) {
        return fg_func(stoi(args[1]));
    }

    perrorSmash("fg", "invalid arguments");
    return COMMAND_FAILURE;
}

int bg(const vector<string> &args) {
    if (args.empty()) return COMMAND_FAILURE;
    if (args.size() == 1) {
        return bg_func(0);
    } else if (args.size() == 2 && is_number(args[1])) {
        return bg_func(stoi(args[1]));
    }

    perrorSmash("bg", "invalid arguments");
    return COMMAND_FAILURE;
}

int quit(const vector<string> &args) {
    if (args.empty()) return COMMAND_FAILURE;
    if (args.size() > 2) {
        perrorSmash("quit", "expected 0 or 1 arguments");
        return COMMAND_FAILURE;
    }
    if (args.size() == 1) { // no kill argument
        return quit_func(false);
    }
    if (args.size() == 2) {
        string killArg = args[1];
        if (strncmp(killArg.c_str(), "kill", 4) == 0) {
            return quit_func(true);
        } else {
            perrorSmash("quit", "unexpected arguments");
            return COMMAND_FAILURE;
        }
    }
    return COMMAND_SUCCESSFUL;
}

int diff(const vector<string> &args) {
    if (args.empty()) return COMMAND_FAILURE;
    if (args.size() != 3 || args[1].empty() || args[2].empty()) {
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

    if (!cwd.empty()) {
        string path(cwd);
        cout << path << endl;
        return COMMAND_SUCCESSFUL;
    }
    return COMMAND_FAILURE;
}

// previous working directory storage
static string prev_wd = "";

// Assumes path string isn't empty
int cd_func(const string path) {
    string curr_wd = get_current_wd_string();
    if (path.compare("-") == 0) {
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
        if (chdir(path.c_str()) != 0) {
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
    jobs_list.garbage_collector(); // clean before printing list
    return jobs_list.print_jobs();
}

int kill_func(int sig_num, int job_id) {
    if (!jobs_list.job_exists(job_id)) {
        string msg = "job id " + to_string(job_id) + " does not exist";
        perrorSmash("kill", msg.c_str());
        return COMMAND_FAILURE;
    }

    pid_t pid = jobs_list.jobs_list.at(job_id).job_pid;

    if (my_system_call(SYS_KILL, pid, sig_num) != 0) {
        perrorSmash("kill", "failed");
        return COMMAND_FAILURE;
    }
    cout << "signal " << sig_num << " was sent to pid " << pid << endl;
    return COMMAND_SUCCESSFUL;
}

int fg_func(int job_id) {
    if (job_id == 0) { // only fg was given
        if (jobs_list.jobs_list.empty()) {
            perrorSmash("fg", "jobs list is empty");
            return COMMAND_FAILURE;
        }

        job_id = jobs_list.get_max_job_id(); // set job_id as max
    }

    // check if job exists
    if (!jobs_list.job_exists(job_id)) {
        string msg = "job id " + to_string(job_id) + " does not exist";
        perrorSmash("fg", msg.c_str());
        return COMMAND_FAILURE;
    }
    // Get job pointer
    Job *job_to_fg = &jobs_list.jobs_list.find(job_id)->second;

    // Get job data
    pid_t pid = job_to_fg->job_pid;
    string cmd_line = job_to_fg->cmd_string;
    int job_state = job_to_fg->job_state;

    // print the job
    cout << "[" << job_id << "] " << cmd_line << endl;

    // send SIGCONT if job is stopped
    if (job_state == STOPPED) {
        if (my_system_call(SYS_KILL, pid, SIGCONT) == -1) {
            perrorSmash("kill", "failed");
            return COMMAND_FAILURE;
        }
    }

    // update foreground process
    fg_process = pid;
    fg_command_str = job_to_fg->cmd_string;
    // remove job from jobs list
    jobs_list.rem_job(job_id);

    // wait for new fg to finish
    if (my_system_call(SYS_WAITPID, pid, NULL, WUNTRACED) == -1) {
        perrorSmash("waitpid", "failed");
        return COMMAND_FAILURE;
    }

    // considering smash calls fg, after new fg process finish the new fg process
    // will be smash
    fg_process = smash_pid;

    return COMMAND_SUCCESSFUL;
}

int bg_func(int job_id) {
    if (job_id == 0) { // only bg without argument

        Job *max_stopped_job = nullptr;

        // check if there is a stopped job, if so set job_id to it
        for (auto it = jobs_list.jobs_list.rbegin();
             it != jobs_list.jobs_list.rend(); ++it) {

            if (it->second.job_state == STOPPED) {
                max_stopped_job = &it->second;
                job_id = it->first; // getting job id by key of the map
                break;
            }
        }

        if (max_stopped_job == nullptr) {
            perrorSmash("bg", "there are no stopped jobs to resume");
            return COMMAND_FAILURE;
        }
    }

    // Check if job exists
    if (!jobs_list.job_exists(job_id)) {
        string msg = "job id " + to_string(job_id) + " does not exist";
        perrorSmash("bg", msg.c_str());
        return COMMAND_FAILURE;
    }

    Job *job_to_cont = &jobs_list.jobs_list.find(job_id)->second;

    // Check if job is stopped
    if (job_to_cont->job_state != STOPPED) {
        string msg = "job id " + to_string(job_id) + " is already in background";
        perrorSmash("bg", msg.c_str());
        return COMMAND_FAILURE;
    }

    cout << job_to_cont->cmd_string << " : " << job_to_cont->job_pid << endl;

    if (my_system_call(SYS_KILL, job_to_cont->job_pid, SIGCONT) == -1) {
        perrorSmash("kill", "failed");
        return COMMAND_FAILURE;
    }

    jobs_list.resume_job(job_to_cont->job_id);
    return COMMAND_SUCCESSFUL;
}

int quit_func(bool kill) {
    if (kill) {
        for (auto const &it: jobs_list.jobs_list) {
            // Get job data
            int job_id = it.first; // jobs_list map key (first) is job_id
            Job job = it.second;
            pid_t pid = job.job_pid;
            string cmd_str = job.cmd_string;

            // Print sigterm msg
            cout << "[" << job_id << "] " << cmd_str << " - sending SIGTERM... "
                 << flush;
            // Send SIGTERM
            if (my_system_call(SYS_KILL, pid, SIGTERM) == -1) {
                perrorSmash("kill", "failed");
            }

            // allow job to wake up and end without violently SIGKILLing it
            if (job.job_state == STOPPED) {
                if (my_system_call(SYS_KILL, pid, SIGCONT) == -1) {
                    perrorSmash("kill", "failed");
                }
            }
            // 5 sec timer
            bool is_killed = false;
            for (int i = 0; i < 5; i++) {
                sleep(1); // sleep 1 sec

                int status;
                pid_t wait_res = my_system_call(SYS_WAITPID, pid, &status, WHOHANG);
                if (wait_res > 0) { // process exited
                    is_killed = true;
                    break;
                } else if (wait_res == -1) { // process no longer exists
                    is_killed = true; // for us it's considered killed
                    break;
                }
                // if the process is still running we continue to wait
            }

            if (is_killed) {
                cout << "done" << endl;
            } else { // process still alive after 5 sec
                cout << "sending SIGKILL... " << flush;
                if (my_system_call(SYS_KILL, pid, SIGKILL) == -1) {
                    perrorSmash("kill", "failed");
                }
                cout << "done" << endl;
            }
        }
    }
    exit(0);
    return COMMAND_SUCCESSFUL; // won't reach but still need a return value
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

    if (res1 == IS_DIR || res2 == IS_DIR) {
        perrorSmash("diff", "paths are not files");
        return COMMAND_FAILURE;
    } else if (res1 == PATH_NOT_EXIST || res2 == PATH_NOT_EXIST) {
        perrorSmash("diff", "expected valid paths for files");
        return COMMAND_FAILURE;
    }

    // open file 1
    fd1 = (int) my_system_call(SYS_OPEN, path1.c_str(), O_RDONLY);
    int err = errno;
    if (fd1 == -1) {
        check_fd_err(err, SYS_OPEN);
        return COMMAND_FAILURE;
    }

    // open file 2
    fd2 = (int) my_system_call(SYS_OPEN, path2.c_str(), O_RDONLY);
    err = errno;
    if (fd2 == -1) {
        check_fd_err(err, SYS_OPEN); // failed to open file2
        int close_res = close_fd(fd1); // close file1
        if (close_res == COMMAND_FAILURE) return COMMAND_FAILURE;
    }

    // read both files and compare
    while (true) {
        // Read chunk from file 1
        bytes_read1 = my_system_call(SYS_READ, fd1, buf1, BUFFER_SIZE);
        err = errno;
        if (bytes_read1 == -1) {
            check_fd_err(err, SYS_READ);
            break;
        }

        // Read chunk from file 2
        bytes_read2 = my_system_call(SYS_READ, fd2, buf2, BUFFER_SIZE);
        err = errno;
        if (bytes_read2 == -1) {
            check_fd_err(err, SYS_READ);
            break;
        }

        // Check if both reads are EOF in the same time
        if (bytes_read1 == 0 && bytes_read2 == 0) {
            result = COMMAND_SUCCESSFUL;
            break;
        }

        // Check if the number of bytes differ
        if (bytes_read1 != bytes_read2) {
            result = COMMAND_FAILURE;
            break;
        }

        // Compare the content of the two buffers
        if (memcmp(buf1, buf2, bytes_read1) != 0) {
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

// ------------- Helper Functions ----------- //

// Checks if a string is a number (for argument checking)
bool is_number(const string &str) {
    if (str.empty()) return false;
    for (char const &c: str) {
        if (isdigit(c) == 0) return false;
    }
    return true;
}

string get_current_wd_string() {
    char buffer[PATH_MAX];

    if (getcwd(buffer, PATH_MAX) != NULL) {
        return string(buffer);
    } else {
        perrorSmash("getcwd", "failed");
        return "";
    }
}
