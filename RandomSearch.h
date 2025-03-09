//
// Created by 28898 on 25-1-3.
//

#ifndef RANDOMSEARCH_H
#define RANDOMSEARCH_H
#include "Job.h"
#include "Schedule.h"


Schedule RandomSearch(const Schedule &schedule, const std::vector<Job> &jobs, const std::vector<std::string> &jobList, double strategy_param, int max_repeat_count, int max_iter_count, int tabu_list_length);

Schedule ConservativeSearch(const Schedule &schedule, const std::vector<Job> &jobs, const std::vector<std::string> &jobList, int tabu_list_length);

Schedule AggressiveSearch(const Schedule &schedule, const std::vector<Job> &jobs, const std::vector<std::string> &jobList);
#endif //RANDOMSEARCH_H
