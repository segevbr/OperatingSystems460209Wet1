#ifndef JOBS_H
#define JOBS_H

#include "globals.h"
#include <string>
#include <iostream>
#include <map>
#include <ctime>
#include <vector>

#define MAX_JOBS 100
using namespace std;

enum Job_state{
    BG = 2, // Resolve confilts with Cmd_err enum
    STOPPED
};

class Job{
    public:
    int job_id;
    string cmd;
    string cmd_string;
    pid_t job_pid;
    int start_time;
    int job_state; // (Readability using Job_state enum)
    
    // C'tor
    Job(const string &cmd, pid_t job_pid) : 
        cmd_string(cmd), job_pid(job_pid), 
        start_time(time(NULL)), job_state(BG){};
    };
    
    class Jobs_list{
        public:
        map<int, Job> jobs_list;
        Jobs_list(){}; // C'tor
        
        // Operations on jobs list
        int add_job(const vector<string> args, pid_t son_pid);
        int rem_job(int job_id);
        int stop_job(int job_id);
        int res_job(int job_id);
        int print_jobs();
        // Helper functions
        int get_min_job_id();
        int get_max_job_id();
        bool job_exists(int job_id);
        int print_job(int job_id);
        int job_runtime(int job_id);
        string get_command_string(const vector<string> args);
};


#endif // JOBS_Hstring cmd_string = "";
