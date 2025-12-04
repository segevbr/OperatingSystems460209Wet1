#include "jobs.h"
#include "my_system_call.h"

// --------- Jobs list functions ---------
// Adding job to list given the cmd and job pid (which should be son's pid)
int Jobs_list::add_job(const vector<string> args, pid_t son_pid) {
    this->garbage_collector();
    if (this->jobs_list.size() < MAX_JOBS) {
        string cmd_str = this->get_command_string(args);
        Job new_job = Job(cmd_str, son_pid);
        if (!args[0].empty()) new_job.cmd = args.at(0);
        new_job.job_id = get_min_job_id();
        this->jobs_list.insert({new_job.job_id, new_job});
        return COMMAND_SUCCESSFUL;
    }
    return COMMAND_FAILURE;
}

// Removes job from jobs list
int Jobs_list::rem_job(int job_id) {
    // if job is in list remove it and delete the job object
    if (job_exists(job_id)) {
        this->jobs_list.erase(job_id);
        return COMMAND_SUCCESSFUL;
    }
    return COMMAND_FAILURE;
}

// Update job state to stopped
int Jobs_list::stop_job(int job_id) {
    if (job_exists(job_id)) {
        this->jobs_list.at(job_id).job_state = STOPPED;
        return COMMAND_SUCCESSFUL;
    }
    return COMMAND_FAILURE;
}

// Update job state to background
int Jobs_list::resume_job(int job_id) {
    if (job_exists(job_id)) {
        this->jobs_list.at(job_id).job_state = BG;
        return COMMAND_SUCCESSFUL;
    }
    return COMMAND_FAILURE;
}

// Print all jobs in jobs list in order of job_id
int Jobs_list::print_jobs() {
    if (jobs_list.empty()) return COMMAND_FAILURE;
    for (int id = 1; id <= MAX_JOBS; id++) {
        if (!job_exists(id)) continue;
        print_job(id);
    }
    return COMMAND_SUCCESSFUL;
}

// Return the minimal possible job_id which can be given to a job
int Jobs_list::get_min_job_id() {
    if (this->jobs_list.empty()) return 1;
    for (int id = 1; id <= MAX_JOBS; id++){
        if (!job_exists(id)) return id; // return the first id who isn't in the list
    }
    return 0;
}

// Return the maximum existing job id
int Jobs_list::get_max_job_id(){
    if (this->jobs_list.empty()) return 1;
    for (int id = MAX_JOBS; id > 0; id--){
        if (job_exists(id)) return id; // return the fisrt id from end which isn't in
                                       // the list
    }
    return 1;
}

// Return true if job exists, otherwise false todo change to is
bool Jobs_list::job_exists(int job_id){
    if (this->jobs_list.find(job_id) != this->jobs_list.end()) return true;
    return false;
}

int Jobs_list::print_job(int job_id) {
    if (!job_exists(job_id)) return COMMAND_FAILURE;
    // Get job data
    string cmd_string = this->jobs_list.at(job_id).cmd_string;
    string job_pid = to_string(this->jobs_list.at(job_id).job_pid);
    string time_elapsed = to_string(job_runtime(job_id));
    string is_stopped = (jobs_list.at(job_id).job_state == STOPPED) ? "(stopped)" : "";

    // print
    cout << "[" << job_id << "] " << cmd_string << ": " << job_pid << " " <<
         time_elapsed << " secs " << is_stopped << endl;

    return COMMAND_SUCCESSFUL;
}

int Jobs_list::job_runtime(int job_id) {
    time_t start = this->jobs_list.at(job_id).start_time;
    time_t end = time(NULL);
    return (int) difftime(end, start);
}

// Helper function to build original command string from string cmd
string Jobs_list::get_command_string(const vector<string> args) {
    string cmd_string = "";
    for (int i = 0; i < int(args.size()); i++) {
        cmd_string += args[i] + " ";
    }
    if (!cmd_string.empty()) cmd_string.pop_back(); // delete last " "
    return cmd_string;
}

void Jobs_list::garbage_collector() {
    int status = 0;
    for (int id = 1; id <= MAX_JOBS; ++id) {
        if (job_exists(id)) {
            auto job = this->jobs_list.at(id);
            if (job.job_state == BG) {
                pid_t pid = job.job_pid;

                long res = my_system_call(SYS_WAITPID, pid, &status, WNOHANG);
                if (res == -1) {
                    perrorSmash("waitpid", "failed");
                    continue;
                }
                if (res > 0) {
                    jobs_list.erase(id);
                }
            }
        }
    }
}