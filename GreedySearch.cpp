//
// Created by luopw on 24-12-1.
//

#include "GreedySearch.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <unordered_set>
#include <sstream>
#include "DataProc.h"

// === 禁忌搜索（Tabu Search） ===
std::unordered_set<std::string> tabu_list;
std::queue<std::string> tabu_queue;
//扰动搜索
Schedule PerturbationSearch(const Schedule &schedule, const std::vector<Job> &jobs,
                            const std::vector<std::string> &jobList) {
    Schedule res_schedule = schedule;
    std::vector<Schedule_item> schedule_items = schedule.get_schedule_items();

    // 执行 2~3 次随机交换扰动
    for (int i = 0; i < 3; i++) {
        int machine1 = rand() % schedule.get_machine_count();
        int machine2 = rand() % schedule.get_machine_count();
        int idx1 = rand() % schedule_items[machine1].process_count;
        int idx2 = rand() % schedule_items[machine2].process_count;

        std::swap(schedule_items[machine1].schedule_process[idx1], schedule_items[machine2].schedule_process[idx2]);

        std::vector<std::vector<int>> temp_graph = ScheduleItemsToGraph(res_schedule, schedule_items, jobs, jobList, false);
        if (hasCycle(temp_graph)) {
            // 如果交换导致循环依赖，则撤销交换
            std::swap(schedule_items[machine1].schedule_process[idx1], schedule_items[machine2].schedule_process[idx2]);
        }
    }

    res_schedule.set_schedule_items(schedule_items);
    return res_schedule;
}


int GetItemIndex(const Schedule &schedule, int job_id, int process_id) {
    for (size_t i = 0; i < schedule.get_schedule_items().size(); i++) {
        for (size_t j = 0; j < schedule.get_schedule_items()[i].process_count; j++) {
            if (schedule.get_schedule_items()[i].schedule_process[j].job_id == job_id &&
                schedule.get_schedule_items()[i].schedule_process[j].process_id == process_id) {
                return j;
            }
        }
    }
    return -1;
}


// 生成 `Schedule` 的哈希值
std::string scheduleToHash(const Schedule &schedule) {
    std::string hash_value;
    for (const auto &item : schedule.get_schedule_items()) {
        hash_value += std::to_string(item.machine_id) + "-";
        for (const auto &proc : item.schedule_process) {
            hash_value += "(" + std::to_string(proc.job_id) + "," + std::to_string(proc.process_id) + ") ";
        }
    }
    return hash_value;
}

// 添加 `Schedule` 到 `tabu_list`
void addTabu(const Schedule &schedule, int tabu_list_length) {
    std::string hash_value = scheduleToHash(schedule);

    if (tabu_list.size() >= tabu_list_length) {
        std::string oldest = tabu_queue.front();
        tabu_queue.pop();
        tabu_list.erase(oldest);
    }

    tabu_list.insert(hash_value);
    tabu_queue.push(hash_value);
}

// 检查 `Schedule` 是否在 `tabu_list` 中
bool isTabu(const Schedule &schedule) {
    return tabu_list.find(scheduleToHash(schedule)) != tabu_list.end();
}

std::vector<std::string> ScoreProcessForExchange(const Schedule &schedule,
                                                 const std::vector<Job> &jobs,
                                                 const std::vector<std::string> &jobList) {
    std::vector<std::string> res_processes;
    std::map<std::string, int> temp_list;

    for (const auto &job : jobList) {
        for (const Job &j : jobs) {
            if (job == j.get_job_name()) {
                for (const auto &process : j.get_job_process()) {
                    std::string process_name = j.get_job_name() + "-" + std::to_string(process.process_item[0].process_id);
                    int item_index = GetItemIndex(schedule, j.get_job_id(), process.process_item[0].process_id);
                    if (item_index != -1) {
                        temp_list[process_name] = item_index;
                    }
                }
            }
        }
    }

    std::vector<std::pair<std::string, int>> temp_vec(temp_list.begin(), temp_list.end());

    std::ranges::sort(temp_vec, [](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
        int a_id = std::stoi(a.first.substr(a.first.find('-') + 1));
        int b_id = std::stoi(b.first.substr(b.first.find('-') + 1));

        return a_id < b_id || (a_id == b_id && a.second > b.second);
    });

    for (const auto &it : temp_vec) {
        res_processes.push_back(it.first);
    }

    return res_processes;
}


// === Greedy Search ===
Schedule GreedySearch(const Schedule &schedule, const std::vector<Job> &jobs,
                      const std::vector<std::string> &jobList, int iter_count) {
    Schedule best_schedule = schedule;
    double best_balance_score = 1e9;
    int tabu_length = 15;  // 初始禁忌表长度

    std::vector<std::string> processlist1 = ScoreProcessForExchange(best_schedule, jobs, jobList);
    bool flag1 = false;

    for (int i = 0; i < iter_count; i++) {
        Schedule temp_schedule = best_schedule;

        // 交替使用 `ExchangeNeighborSearch()` 和 `MoveNeighborSearch()`
        if (i % 2 == 0) {
            temp_schedule = ExchangeNeighborSearch(best_schedule, jobs, jobList, processlist1, flag1);
        } else {
            temp_schedule = MoveNeighborSearch(best_schedule, jobs, jobList, processlist1, flag1);
        }

        // 执行 `PerturbationSearch()` 以打破局部最优
        if (i % 10 == 0) {
            temp_schedule = PerturbationSearch(temp_schedule, jobs, jobList);
        }

        // 计算任务均衡性评分
        double avg_jobs = 0;
        for (const auto &item : temp_schedule.get_schedule_items()) {
            avg_jobs += item.process_count;
        }
        avg_jobs /= temp_schedule.get_machine_count();

        double balance_score = 0;
        for (const auto &item : temp_schedule.get_schedule_items()) {
            balance_score += std::abs(item.process_count - avg_jobs);
        }
        balance_score /= temp_schedule.get_machine_count();

        // 动态调整 `tabu_list` 长度
        if (i > 0 && i % 5 == 0) {
            if (temp_schedule.get_TotalTime() == best_schedule.get_TotalTime()) {
                tabu_length = std::min(tabu_length + 5, 30);
            } else {
                tabu_length = std::max(tabu_length - 5, 10);
            }
        }

        if (!isTabu(temp_schedule)) {
            addTabu(temp_schedule, tabu_length);
            if (temp_schedule.get_TotalTime() < best_schedule.get_TotalTime()) {
                best_schedule = temp_schedule;
            } else if (balance_score < best_balance_score) {
                best_schedule = temp_schedule;
                best_balance_score = balance_score;
            }
        }
    }

    return best_schedule;
}


// === Exchange Neighbor Search ===
Schedule ExchangeNeighborSearch(const Schedule &schedule, const std::vector<Job> &jobs,
                                const std::vector<std::string> &jobList,
                                std::vector<std::string> &processlist, bool &flag) {
    // 复制当前调度方案
    Schedule res_schedule = schedule;
    res_schedule.set_schedule_id(-1);

    std::vector<Schedule_item> schedule_items = schedule.get_schedule_items();
    std::string temp_process = "";

    // 遍历所有可交换的任务
    for (auto process : processlist) {
        int machine_id = -1, item_id = -1;

        // 获取该任务所在的机器ID和位置索引
        GetMachineIdAndItemIdByProcess(schedule_items, process, 1, machine_id, item_id);

        // 如果任务在机器上的索引有效
        if (item_id > 0) {
            // 交换相邻的两个任务
            std::swap(schedule_items[machine_id].schedule_process[item_id],
                      schedule_items[machine_id].schedule_process[item_id - 1]);

            // 生成新的邻接矩阵，检查是否有环
            std::vector<std::vector<int>> graph1 = ScheduleItemsToGraph(res_schedule, schedule_items, jobs, jobList, false);

            // 如果新图中有环，则撤销交换
            if (hasCycle(graph1)) {
                std::swap(schedule_items[machine_id].schedule_process[item_id],
                          schedule_items[machine_id].schedule_process[item_id - 1]);
                continue;
            }

            // 更新调度方案并计算新的 `Total time`
            res_schedule.set_graph(graph1);
            int new_total_time = CalculateTotalTime(res_schedule);

            // 只有当新调度方案的 `Total time` 下降时，才应用交换
            if (new_total_time < schedule.get_TotalTime()) {
                res_schedule.set_schedule_id(schedule.get_schedule_id() + 1);
                res_schedule.set_machine_count(schedule.get_machine_count());
                res_schedule.set_schedule_items(schedule_items);
                temp_process = process;
                flag = true;  // 标记交换成功
                break;  // 交换成功后，退出循环
            } else {
                // `Total time` 没有下降，撤销交换
                std::swap(schedule_items[machine_id].schedule_process[item_id],
                          schedule_items[machine_id].schedule_process[item_id - 1]);
            }
        }
    }

    // 如果 `processlist` 里有这个任务，删除它
    auto it = std::remove(processlist.begin(), processlist.end(), temp_process);
    if (it != processlist.end()) {
        processlist.erase(it, processlist.end());
    }

    return res_schedule;
}

bool CheckProcessOrder(const std::vector<Schedule_item> &schedule_items) {
    for (const auto &schedule_item : schedule_items) {
        int last_process_id = -1;
        for (const auto &proc : schedule_item.schedule_process) {
            if (proc.process_id <= last_process_id) {
                return false;  // **如果 `process_id` 变小了，说明顺序错误**
            }
            last_process_id = proc.process_id;
        }
    }
    return true;
}


Schedule MoveNeighborSearch(const Schedule &schedule, const std::vector<Job> &jobs,
                            const std::vector<std::string> &jobList, std::vector<std::string> &processlist, bool &flag) {
    // 复制当前调度方案
    Schedule res_schedule = schedule;
    res_schedule.set_schedule_id(-1);
    std::vector<Schedule_item> schedule_items = schedule.get_schedule_items();

    int machine_id = -1, item_id = -1, job_id = 0, process_id = 0;
    std::string temp_process = "";

    for (auto process : processlist) {
        GetMachineIdAndItemIdByProcess(schedule_items, process, 2, machine_id, item_id);

        if (item_id >= 0) {
            job_id = schedule_items[machine_id].schedule_process[item_id].job_id;
            process_id = schedule_items[machine_id].schedule_process[item_id].process_id;

            Job selected_job = SelectJobByJobId(jobs, job_id);
            if (selected_job.get_job_id() == 0 || selected_job.get_process_count() <= process_id) {
                continue;  // 避免越界访问
            }
            Job_process temp_job_process = selected_job.get_job_process()[process_id];

            // 选择负载最轻的可用机器
            int min_process_count = schedule_items[machine_id].process_count;
            int best_machine_id = machine_id;

            for (const auto &process_item : temp_job_process.process_item) {
                int alt_machine_id = process_item.machine_id;
                int alt_process_count = GetProcessCountByMachineId(schedule, alt_machine_id);

                if (alt_process_count >= 0 && alt_process_count < min_process_count) {
                    min_process_count = alt_process_count;
                    best_machine_id = alt_machine_id;
                }
            }

            // 只有当 `best_machine_id` 不等于 `machine_id` 时才移动任务
            if (best_machine_id != machine_id) {
                std::vector<Schedule_item> temp_schedule_items = schedule_items;

                // 移除任务
                temp_schedule_items[machine_id].schedule_process.erase(
                        temp_schedule_items[machine_id].schedule_process.begin() + item_id);
                temp_schedule_items[machine_id].process_count--;

                // 插入任务到新机器
                Schedule_process new_process;
                new_process.job_id = job_id;
                new_process.process_id = process_id;
                temp_schedule_items[best_machine_id].schedule_process.push_back(new_process);
                temp_schedule_items[best_machine_id].process_count++;

                // 生成 `graph` 并检查是否有环
                std::vector<std::vector<int>> temp_graph = ScheduleItemsToGraph(res_schedule, temp_schedule_items, jobs, jobList, false);

                if (!hasCycle(temp_graph) && CheckProcessOrder(temp_schedule_items)) {
                    // **如果没有形成环，则应用变更**
                    schedule_items = temp_schedule_items;
                    res_schedule.set_graph(temp_graph);
                    res_schedule.set_schedule_items(schedule_items);

                    int new_total_time = CalculateTotalTime(res_schedule);
                    if (new_total_time <= schedule.get_TotalTime()) {
                        res_schedule.set_schedule_id(schedule.get_schedule_id() + 1);
                        res_schedule.set_machine_count(schedule.get_machine_count());
                        temp_process = process;
                        flag = true;
                        break;
                    }
                }

                // **如果形成环，撤销变更**
                schedule_items = schedule.get_schedule_items();
            }
        }
    }

    auto it = std::remove(processlist.begin(), processlist.end(), temp_process);
    if (it != processlist.end()) {
        processlist.erase(it, processlist.end());
    }

    return res_schedule;
}



Job SelectJobByJobId(const std::vector<Job> &jobs, const int job_id) {
    for (auto job: jobs) {
        if (job.get_job_id() == job_id) {
            return job;
        }
    }
    return Job();
}

int GetProcessCountByMachineId(const Schedule &schedule, const int machine_id) {
    for (int i = 0; i<schedule.get_schedule_items().size(); i++) {
        if (schedule.get_schedule_items()[i].machine_id == machine_id) {
            return schedule.get_schedule_items()[i].process_count;
        }
    }
    return 0;
}



