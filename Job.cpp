//
// Created by luopw on 24-12-1.
//

#include "Job.h"
#include <iostream>


Job::Job() {
    job_id = 0;
    job_name = "";
    process_count = 0;
    job_process.clear();
}
int Job::get_job_id() const {return job_id;}
std::string Job::get_job_name() const {return job_name;}
int Job::get_process_count() const {return process_count;}
const std::vector<Job_process>& Job::get_job_process() const {
    return job_process;
}
void Job::set_job_id(const int job_id) {this->job_id = job_id;}
void Job::set_job_name(const std::string &job_name) {this->job_name = job_name;}
void Job::set_process_count(const int process_count) {this->process_count = process_count;}
void Job::set_job_process(const std::vector<Job_process> &job_process) {this->job_process = job_process;}
void Job::to_string() {
    std::cout << "job_id: " << job_id << ", job_name: " << job_name << ", process_count: " << process_count << std::endl;
    for (auto &[machine_count, process_item] : job_process) {
        std::cout << "machine_count: " << machine_count << std::endl;
        for (auto &[process_id, machine_id, process_time] : process_item) {
            std::cout << "process_id: " << process_id << ", machine_id: " << machine_id << ", process_time: " << process_time << std::endl;
        }
    }
}
