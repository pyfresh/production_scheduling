#include <iostream>

#include "DataProc.h"
#include "GreedySearch.h"
#include "RandomSearch.h"

int main() {
    auto machines = std::vector<Machine>();      // 机器列表
    auto jobs = std::vector<Job>();              // 工件列表
    auto orders = std::vector<Order>();          // 订单列表
    Init(machines, jobs, orders);    // 初始化机器、工件、订单
    auto jobList = std::vector<std::string>();   // 用于记录每个工件列表，根据订单生成
    OrderToJobList(orders, jobList);         // 将订单转换为待加工的工件列表
    Schedule input_schedule;                    // 输入的调度方案
    Schedule output_schedule;                   // 输出的调度方案

    //parameters
    double search_mode_param = 1.0;  // 搜索模式参数
    double random_search_strategy_param = 0.5;  // 搜索模式参数
    int max_iter_count = 20;  // 最大迭代次数
    int tabu_list_length = 10;  // 禁忌表长度

    // 生成初始解
    Schedule schedule0 = GenerateInitialSolution(jobList, jobs, machines);
    schedule0.to_string();
    // schedule0.ToMapCode();
    const int ans = CalculateTotalTime(schedule0);
    std::cout << "Total time: " << ans << std::endl;

    Schedule temp_schedule;
    input_schedule = schedule0;
    output_schedule = schedule0;
    // while (CheckStopFlag()) {
    //     double random_num = static_cast<double>(std::rand()) / RAND_MAX;
    //     if (random_num < search_mode_param) {
    //         // 选择随机搜索模式
    //         temp_schedule = RandomSearch(input_schedule, jobs, jobList, random_search_strategy_param, max_repeat_count, max_iter_count, tabu_list_length);
    //         if (temp_schedule.get_TotalTime()<output_schedule.get_TotalTime()) {
    //             output_schedule = temp_schedule;
    //         }
    //     } else {
    //         // 选择贪心搜索模式
    //         temp_schedule = GreedySearch(input_schedule, jobs, jobList, max_iter_count);
    //         if (temp_schedule.get_TotalTime()<output_schedule.get_TotalTime()) {
    //             output_schedule = temp_schedule;
    //             input_schedule = temp_schedule;
    //         }
    //         else {
    //             // 选择随机搜索模式
    //             input_schedule = RandomSearch(input_schedule, jobs, jobList, random_search_strategy_param, max_repeat_count, max_iter_count, tabu_list_length);
    //             if (input_schedule.get_TotalTime()<output_schedule.get_TotalTime()) {
    //                 output_schedule = temp_schedule;
    //             }
    //         }
    //     }
    // }

    int search_time = time(nullptr), count = 0, repeat_count =0;
    while (CheckStopFlag(search_time, count, repeat_count)) {
        temp_schedule = GreedySearch(input_schedule, jobs, jobList, max_iter_count);
        if (temp_schedule.get_TotalTime()<=output_schedule.get_TotalTime()) {
            if (temp_schedule.get_TotalTime() < output_schedule.get_TotalTime()) {
                repeat_count = 1;
            } else {
                repeat_count++;
            }
            output_schedule = temp_schedule;
            input_schedule = temp_schedule;
            std::cout << "Total time: " << CalculateTotalTime(temp_schedule) << std::endl;
        }
        count++;
    }

    // output_schedule = GreedyAdjust(schedule0, jobs, jobList);
    output_schedule.to_string();
    // output_schedule.ToMapCode();
    const int ans1 = CalculateTotalTime(output_schedule);
    std::cout << "Total time: " << ans1 << std::endl;

    return 0;
}
