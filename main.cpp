#include <iostream>
#include <fstream>
#include <vector>
#include "DataProc.h"
#include "ImprovedSearch.h"
#include "GreedySearch.h"
#include "RandomSearch.h"

void save_data_to_file(const std::vector<int>& greedy_times, const std::vector<int>& improved_times) {
    // 创建输出文件
    std::ofstream out_file("scheduling_times.txt");

    if (!out_file) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    // 写入列头
    out_file << "Greedy Algorithm,Improved Algorithm" << std::endl;

    // 写入每一轮的结果
    size_t min_size = std::min(greedy_times.size(), improved_times.size());
    for (size_t i = 0; i < min_size; ++i) {
        out_file << greedy_times[i] << "," << improved_times[i] << std::endl;
    }

    out_file.close();
}

int main() {
    std::vector<int> greedy_times;   // 存储贪心算法的结果
    std::vector<int> improved_times; // 存储改进算法的结果

    //parameters
    int max_iter_count = 20;  // 最大迭代次数
    int tabu_list_length = 10;  // 禁忌表长度

    // 初始化机器、工件、订单
    auto machines = std::vector<Machine>();
    auto jobs = std::vector<Job>();
    auto orders = std::vector<Order>();
    Init(machines, jobs, orders);    // 初始化机器、工件、订单

    auto jobList = std::vector<std::string>();   // 用于记录每个工件列表，根据订单生成
    OrderToJobList(orders, jobList);         // 将订单转换为待加工的工件列表

    Schedule input_schedule;                    // 输入的调度方案
    Schedule output_schedule;                   // 输出的调度方案

    // 生成初始解
    Schedule schedule0 = GenerateInitialSolution(jobList, jobs, machines);
    schedule0.to_string();
    const int ans = CalculateTotalTime(schedule0);
    std::cout << "Total time: " << ans << std::endl;

    Schedule temp_schedule;
    input_schedule = schedule0;  // 重置为初始解
    output_schedule = schedule0;

    int search_time = time(nullptr), count = 0, repeat_count = 0;

    // 执行 GreedySearch
    while (CheckStopFlag(search_time, count, repeat_count)) {
        temp_schedule = GreedySearch(input_schedule, jobs, jobList, max_iter_count);
        if (temp_schedule.get_TotalTime() <= output_schedule.get_TotalTime()) {
            if (temp_schedule.get_TotalTime() < output_schedule.get_TotalTime()) {
                repeat_count = 1;
            } else {
                repeat_count++;
            }
            output_schedule = temp_schedule;
            input_schedule = temp_schedule;  // 此处更新 input_schedule 为 GreedySearch 的结果
            std::cout << "Total time (Greedy): " << CalculateTotalTime(temp_schedule) << std::endl;

            // 将 Greedy 算法的结果保存
            greedy_times.push_back(CalculateTotalTime(temp_schedule));
        }

        count++;
    }

    // 重新初始化机器、工件、订单并重新生成初始解
    machines = std::vector<Machine>();
    jobs = std::vector<Job>();
    orders = std::vector<Order>();
    Init(machines, jobs, orders);    // 初始化机器、工件、订单
    jobList = std::vector<std::string>();   // 用于记录每个工件列表，根据订单生成
    OrderToJobList(orders, jobList);         // 将订单转换为待加工的工件列表

    Schedule schedulenew = GenerateInitialSolution(jobList, jobs, machines);
    schedulenew.to_string();
    const int ansnew = CalculateTotalTime(schedulenew);
    std::cout << "Total time: " << ansnew << std::endl;

    input_schedule = schedulenew;  // 重置为初始解
    output_schedule = schedulenew;

    // 执行 ImprovedSearch
    search_time = time(nullptr), count = 0, repeat_count = 0;
    while (CheckStopFlag(search_time, count, repeat_count)) {
        temp_schedule = ImprovedSearch(input_schedule, jobs, jobList, max_iter_count);
        if (temp_schedule.get_TotalTime() <= output_schedule.get_TotalTime()) {
            if (temp_schedule.get_TotalTime() < output_schedule.get_TotalTime()) {
                repeat_count = 1;
            } else {
                repeat_count++;
            }
            output_schedule = temp_schedule;
            input_schedule = temp_schedule;  // 此处更新 input_schedule 为 ImprovedSearch 的结果
            std::cout << "Total time (Improved): " << CalculateTotalTime(temp_schedule) << std::endl;

            // 将 Improved 算法的结果保存
            improved_times.push_back(CalculateTotalTime(temp_schedule));
        }

        count++;
    }

    // 输出最终调度
    output_schedule.to_string();
    const int ans1 = CalculateTotalTime(output_schedule);
    std::cout << "Total time (Final): " << ans1 << std::endl;

    // 将数据保存到文件
    save_data_to_file(greedy_times, improved_times);

    return 0;
}
