//
// Created by 28898 on 25-1-3.
//

#include "RandomSearch.h"


Schedule RandomSearch(const Schedule &schedule, const std::vector<Job> &jobs, const std::vector<std::string> &jobList, const double strategy_param, const int max_repeat_count, int max_iter_count, const int tabu_list_length) {
    std::vector<Schedule> scheduleList;
    Schedule best_schedule = schedule;
    int repeat_count = 0;  // 最优解的重复次数
    const double random_num = static_cast<double>(std::rand()) / RAND_MAX;
    if (random_num < (repeat_count/max_repeat_count) - strategy_param) {
        while (scheduleList.size() < max_iter_count) {
            scheduleList.push_back(ConservativeSearch(best_schedule, jobs, jobList, tabu_list_length));
        }
        for (Schedule temp_schedule: scheduleList) {
            if (temp_schedule.get_TotalTime() < best_schedule.get_TotalTime()) {
                best_schedule = temp_schedule;
            }
        }
    }
    else {
        best_schedule = AggressiveSearch(best_schedule, jobs, jobList);
    }
    return best_schedule;
}

Schedule ConservativeSearch(const Schedule &schedule, const std::vector<Job> &jobs, const std::vector<std::string> &jobList, int tabu_list_length) {
    Schedule res_schedule;
    // TODO: ConservativeSearch, 保守搜索，使用禁忌搜索算法，对调度方案进行调整，返回调整后的调度方案列表

    return res_schedule;
}

Schedule AggressiveSearch(const Schedule &schedule, const std::vector<Job> &jobs, const std::vector<std::string> &jobList) {
    Schedule res_schedule;
    // TODO: AggressiveSearch, 激进搜索，对调度方案进行调整，返回调整后的调度方案

    return res_schedule;
}