// signals.c
#include "signals.h"
#include "commands.h"
#include "globals.h"
#include "jobs.h"
#include <iostream>
#include <unistd.h>

using namespace std;
extern Jobs_list jobs_list;
extern string fg_command_str;

void catch_ctrl_c(int signum) {
    cout << "smash: caught CTRL+C" << endl;

    if (fg_process != smash_pid){ // if a process other than smash is running in fg
        if (my_system_call(SYS_KILL, fg_process, SIGKILL) == 0){
            cout << "smash: process " << fg_process << " was killed" << endl;
            fg_process = smash_pid;
        } else {
            perrorSmash("kill", "faile");
        }
    }
}

void catch_ctrl_z(int signum) {
    cout << "smash: caught CTRL-Z" << endl;
    if (fg_process != smash_pid){
        if (my_system_call(SYS_KILL, fg_process, SIGSTOP) == 0){
            cout << "smash: process " << fg_process << " was stopped" << endl;
            
            if (!fg_command_str.empty()){ 
                vector<string> args;
                args.push_back(fg_command_str);
                jobs_list.add_job(args, fg_process);
                int job_id = jobs_list.get_job_id_from_pid(fg_process);
                jobs_list.stop_job(job_id); 
            }

            // update fg_process to be smash
            fg_process = smash_pid;
        } else {
            perrorSmash("kill", "failed");
        }
    }
}


void catch_sigchld(int signum) {
    int status;
    pid_t pid;

    // -1 tells waitpid to wait for any child process
    while ((pid = my_system_call(SYS_WAITPID, -1, &status, WNOHANG)) > 0){
        // Only attempt to remove the job if its PID was found.
        int job_id = jobs_list.get_job_id_from_pid(pid);
        
        if (job_id != -1) {
            jobs_list.rem_job(job_id);
        }
    }
}