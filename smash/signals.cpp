// signals.c
#include "signals.h"
#include "commands.h"
#include "globals.h"
#include "jobs.h"
#include <iostream>
#include <unistd.h>

using namespace std;
extern Jobs_list jobs_list;
 
void catch_sigchld(int signum) {
    int status;
    pid_t pid;

    // Remove all zombies (all child processes which ended) without blocking samsh
    while ((pid = my_system_call(SYS_WAITPID, -1, &status, WNOHANG)) > 0){
        int job_id = jobs_list.get_job_id_from_pid(pid);
        jobs_list.rem_job(job_id);
    }
}

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
        if (my_system_call(SYS_KILL, fg_process, SIGTSTP) == 0){
            cout << "smash: process " << fg_process << " was stopped" << endl;
            // int job_id = jobs_list.get_job_id_from_pid(fg_process);
            // stop job in list?
            fg_process = smash_pid;
        } else {
            perrorSmash("kill", "failed");
        }
    }
}