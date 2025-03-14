//
// Created by luopw on 24-12-1.
//

#include "DataProc.h"
#include "Schedule.h"

#include <fstream>
#include <iostream>
#include <queue>


void Init(std::vector<Machine> &machines, std::vector<Job> &jobs, std::vector<Order> &orders) {
    // // 重定向输入流为当前目录下的input.txt文件,使得使用std::cin读取文件内容，使用std::cout输出到控制台
    std::string inputFilePath = "C:\\Users\\Litbug\\Desktop\\production_scheduling\\input.txt";
    std::ifstream in(inputFilePath);
    std::cin.rdbuf(in.rdbuf());
    if (!in) {
        std::cerr << "Error: Cannot open input file." << std::endl;
        return;
    }

    int machineNum=0, jobNum=0, maxProcessCount=0;
    std::cin >> jobNum >>machineNum >> maxProcessCount;
    // std::cout << "jobNum: " << jobNum << ", machineNum: " << machineNum << std::endl;
    for (int i = 0; i < jobNum; i++) {
        Job jobItem;
        int processCount;
        std::cin >> processCount;
        jobItem.set_process_count(processCount);
        jobItem.set_job_id(i);
        jobItem.set_job_name("job" + std::to_string(i));
        std::vector<Job_process> jobProcess;
        for (int j=0; j<processCount; j++) {
            Job_process jobProcessItem;
            int machineCount;
            std::cin >> machineCount;
            jobProcessItem.machine_count = machineCount;
            for (int k=0; k<machineCount; k++) {
                Process_item processItem;
                std::cin >> processItem.machine_id >> processItem.process_time;
                processItem.process_id = j;
                jobProcessItem.process_item.push_back(processItem);
            }
            jobProcess.push_back(jobProcessItem);
        }
        jobItem.set_job_process(jobProcess);
        jobs.push_back(jobItem);
    }
    for (int i = 0; i < machineNum; i++) {
        Machine machineItem;
        machineItem.set_machine_id(i);
        machineItem.set_machine_name("machine" + std::to_string(i));
        machineItem.set_machine_type(Machine_type::Job_shop_multi_process_machine);
        machineItem.set_count(1);
        machineItem.set_idle_count(1);
        machines.push_back(machineItem);
    }
    std::vector<Order_item> orderItems;
    for (int i = 0; i < jobNum; i++) {
        Order_item orderItemItem;
        orderItemItem.job_name = "job" + std::to_string(i);
        orderItemItem.job_count = 1;
        orderItems.push_back(orderItemItem);
    }
    Order orderItem;
    orderItem.set_order_id("order0");
    orderItem.set_order_items(orderItems);
    orders.push_back(orderItem);
}

void OrderToJobList(const std::vector<Order> &orders, std::vector<std::string> &jobList) {
    for (auto &orderItem : orders) {
        for (auto &[job_name, job_count] : orderItem.get_order_items()) {
            for (int i=0;i<job_count;i++) {
                jobList.push_back(job_name);
            }
        }
    }
}

Schedule GenerateInitialSolution(const std::vector<std::string> &jobList, const std::vector<Job> &jobs, std::vector<Machine> &machines) {
    Schedule schedule;
    const int machineNum = machines.size();
    schedule.set_machine_count(machineNum);
    auto temp_schedule_items = std::vector<Schedule_item>();
    for (int i = 0; i < machineNum; i++) {
        Schedule_item scheduleItem;
        scheduleItem.machine_id = i;
        scheduleItem.process_count = 0;
        scheduleItem.schedule_process.clear();
        temp_schedule_items.push_back(scheduleItem);
    }
    schedule.set_schedule_items(temp_schedule_items);

    for (auto &jobName : jobList) {
        // std::cout << "jobName: " << jobName << std::endl;
        auto job = SelectJobByJobName(jobs, jobName);
        // job.to_string();
        if (job.get_job_name() == "") {
            std::cout << "Error: Cannot find job by job name." << std::endl;
            return Schedule();
        }
        for (auto &[machine_count, process_item] : job.get_job_process()) {
            // 遍历获取工序数量最少的机器的id
            int temp_machine_id = process_item[0].machine_id;
            int machine_with_min_process_count = machines[temp_machine_id].get_process_count();
            for (int i = 1; i < process_item.size(); i++) {
                for (auto machine : machines) {
                    if (machine.get_machine_id() == process_item[i].machine_id) {
                        if (machine.get_process_count() < machine_with_min_process_count) {
                            temp_machine_id = process_item[i].machine_id;
                            machine_with_min_process_count = machine.get_process_count();
                        }
                    }
                }
            }
            schedule.AddProcess(temp_machine_id, job.get_job_id(), process_item[0].process_id);
            // 将temp_machine_id对应的机器的process_count加1
            for (auto &machine : machines) {
                if (machine.get_machine_id() == temp_machine_id) {
                    machine.set_process_count(machine.get_process_count() + 1);
                    break;
                }
            }
        }
    }
    std::vector<Schedule_item> temp_items = schedule.get_schedule_items();
    std::vector<std::vector<int>> graph = ScheduleItemsToGraph(schedule, temp_items, jobs, jobList, true);
    schedule.set_schedule_id(0);
    return schedule;
}

Job SelectJobByJobName(const std::vector<Job> &jobs, const std::string &jobName) {
    const std::string temp = jobName.substr(0, jobName.find('-'));
    for (auto &job : jobs) {
        if (job.get_job_name() == temp) {
            return job;
        }
    }
    return Job();
}

int GetProcessTime(const std::vector<Job> &jobs, const int machine_id, const int job_id, const int process_id) {
    for (auto job : jobs) {
        if (job.get_job_id() == job_id) {
            for (auto &[machine_count, process_item] : job.get_job_process()) {
                for (auto &[temp_process_id, temp_machine_id, temp_process_time] : process_item) {
                    if (machine_id == temp_machine_id && process_id == temp_process_id) {
                        return temp_process_time;
                    }
                }
            }
        }
    }
    return 0;
}

std::vector<std::vector<int>> ScheduleItemsToGraph(Schedule &schedule, std::vector<Schedule_item> &schedule_items,
                                                   const std::vector<Job> &jobs, const std::vector<std::string> &jobList,
                                                   bool flag) {
    std::vector<std::string> processList;
    processList.push_back("start");  // 添加起始节点

    for (auto &schedule_item : schedule_items) {
        for (auto &proc : schedule_item.schedule_process) {
            processList.push_back(std::to_string(proc.job_id) + "-" + std::to_string(proc.process_id));
        }
    }

    processList.push_back("end");  // 添加结束节点
    int processCount = processList.size();

    // 生成邻接矩阵，初始化为 -1
    std::vector<std::vector<int>> graph(processCount, std::vector<int>(processCount, -1));

    int index1 = 0, index2 = 0;

    // 遍历 `schedule_items`，生成机器上的依赖关系
    for (const auto &schedule_item : schedule_items) {
        for (size_t i = 0; i < schedule_item.schedule_process.size(); i++) {
            int job_id = schedule_item.schedule_process[i].job_id;
            int process_id = schedule_item.schedule_process[i].process_id;

            // 找到 `index1`
            for (size_t k = 0; k < processList.size(); k++) {
                if (processList[k] == std::to_string(job_id) + "-" + std::to_string(process_id)) {
                    index1 = k;
                    break;
                }
            }

            // 连接 `start -> first process`
            if (i == 0) {
                graph[0][index1] = 0;
            }

            // 连接 `last process -> end`
            if (i == schedule_item.schedule_process.size() - 1) {
                graph[index1][processCount - 1] = GetProcessTime(jobs, schedule_item.machine_id, job_id, process_id);
            }

            // 连接 `previous process -> current process`
            if (i > 0) {
                int prev_job_id = schedule_item.schedule_process[i - 1].job_id;
                int prev_process_id = schedule_item.schedule_process[i - 1].process_id;

                for (size_t k = 0; k < processList.size(); k++) {
                    if (processList[k] == std::to_string(prev_job_id) + "-" + std::to_string(prev_process_id)) {
                        index2 = k;
                        break;
                    }
                }

                graph[index2][index1] = GetProcessTime(jobs, schedule_item.machine_id, job_id, process_id);
            }
        }
    }

    // 遍历 `jobList`，添加工序之间的约束
    for (const auto &job : jobList) {
        for (const auto &jobItem : jobs) {
            if (jobItem.get_job_name() == job) {
                int jobProcessCount = jobItem.get_process_count();
                int job_id = jobItem.get_job_id();

                for (int i = 0; i < jobProcessCount - 1; i++) {
                    int process_id1 = i;
                    int process_id2 = i + 1;

                    for (size_t k = 0; k < processList.size(); k++) {
                        if (processList[k] == std::to_string(job_id) + "-" + std::to_string(process_id1)) {
                            index1 = k;
                        }
                        if (processList[k] == std::to_string(job_id) + "-" + std::to_string(process_id2)) {
                            index2 = k;
                        }
                    }

                    graph[index1][index2] = GetProcessTime(jobs, job_id, job_id, process_id1);
                }
            }
        }
    }

    // **检查是否形成环**
//    if (hasCycle(graph)) {
//        throw std::runtime_error("Error: Graph contains a cycle and cannot be topologically sorted.");
//    }

    // 更新 `schedule`
    if (flag) {
        schedule.set_graph(graph);
        schedule.set_processList(processList);
        CalculateTotalTime(schedule);
    }

    return graph;
}


int CalculateTotalTime(Schedule &schedule) {
    int ans = 0;
    const std::vector<std::vector<int>> graph = schedule.get_graph();
    const int processCount = graph.size();
    std::vector<int> inDegree(processCount, 0);  // 记录每个节点的入度
    for (int i = 0; i<processCount; i++) {
        for (int j = 0; j<processCount; j++) {
            if (graph[i][j] != -1) {
                inDegree[j]++;
            }
        }
    }
    // 获取拓扑排序的列表
    std::vector<int> toPoSort;
    std::queue<int> zeroInDegreeQueue;
    for (int i = 0; i<processCount; i++) {
        if (inDegree[i] == 0) {
            zeroInDegreeQueue.push(i);
        }
    }
    while (!zeroInDegreeQueue.empty()) {
        int temp = zeroInDegreeQueue.front();
        zeroInDegreeQueue.pop();
        toPoSort.push_back(temp);
        for (int i = 0; i<processCount; i++) {
            if (graph[temp][i] != -1) {
                inDegree[i]--;
                if (inDegree[i] == 0) {
                    zeroInDegreeQueue.push(i);
                }
            }
        }
    }
    // std::cout << "Topological sort: ";
    // for (int i = 0; i<toPoSort.size(); i++) {
    //     std::cout << toPoSort[i] << " ";
    // }
    // std::cout << std::endl;

    // 检查是否存在环
    if (toPoSort.size() != processCount) {
        throw std::runtime_error("calculate time: The graph contains a cycle and cannot be topologically sorted.");
    }

    std::vector<int> startTime(processCount, 0);
    for (int i =0 ;i<toPoSort.size(); i++) {
        for (int j = 0; j<processCount; j++) {
            if (graph[j][toPoSort[i]] != -1) {
                startTime[toPoSort[i]] = std::max(startTime[toPoSort[i]], startTime[j] + graph[j][toPoSort[i]]);
            }
        }
    }

    ans = startTime[processCount-1];
    schedule.set_start_time(startTime);
    schedule.set_TotalTime(ans);
    return ans;
}

// 设置停止标志，当搜索时间超过指定时间或搜索次数超过指定次数时，停止搜索
bool CheckStopFlag(const int &search_time, const int &count, int repeat_count) {
    if (const int now = time(nullptr); now - search_time > 300) {
        return false;
    }
    if (count > 1000) {
        return false;
    }
    if (repeat_count>10) {
        return false;
    }
    return true;
}


void GetMachineIdAndItemIdByProcess(const std::vector<Schedule_item> &schedule_items,
                                    const std::string &process, int type, int &x, int &y) {
    for (size_t i = 0; i < schedule_items.size(); i++) {
        for (size_t j = 0; j < schedule_items[i].process_count; j++) {
            if (type == 1) {
                if (process == "job" + std::to_string(schedule_items[i].schedule_process[j].job_id) +
                               "-" + std::to_string(schedule_items[i].schedule_process[j].process_id)) {
                    x = i;
                    y = j;
                    return;
                }
            } else if (type == 2) {
                if (process == std::to_string(schedule_items[i].schedule_process[j].job_id) +
                               "-" + std::to_string(schedule_items[i].schedule_process[j].process_id)) {
                    x = i;
                    y = j;
                    return;
                }
            }
        }
    }
}

bool hasCycle(const std::vector<std::vector<int>> &adjMatrix) {
    size_t n = adjMatrix.size();
    std::vector<int> inDegree(n, 0);

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            if (adjMatrix[i][j] != -1) {
                inDegree[j]++;
            }
        }
    }

    std::queue<int> q;
    for (size_t i = 0; i < n; i++) {
        if (inDegree[i] == 0) {
            q.push(i);
        }
    }

    int visitedCount = 0;
    while (!q.empty()) {
        int node = q.front();
        q.pop();
        visitedCount++;

        for (size_t j = 0; j < n; j++) {
            if (adjMatrix[node][j] != -1) {
                inDegree[j]--;
                if (inDegree[j] == 0) {
                    q.push(j);
                }
            }
        }
    }

    return visitedCount != n;
}




