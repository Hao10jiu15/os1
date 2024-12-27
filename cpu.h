// cpu.h
#ifndef CPU_H
#define CPU_H

#include "pcb.h"
#include "allhead.h" // 包含 allhead.h 获取 ALL_MEMORY_SIZE
#include <unordered_map>
#include <chrono>
#include <thread>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <sstream>
#include <limits>

// 假设 code 是全局变量，用于存储所有进程的指令
extern std::vector<std::string> code;

// 辅助函数，用于移除末尾的逗号
std::string stripComma(const std::string &str)
{
    if (!str.empty() && str.back() == ',')
        return str.substr(0, str.size() - 1);
    return str;
}

class CPU
{
public:
    CPU(int _timeSlice)
        : timeSlice(_timeSlice),
          currentProcess(nullptr),
          inputAvailable(false),
          lastInstructionTime(std::chrono::steady_clock::now()) // 初始化上一次指令执行时间为当前时间
    {
    }

    void addProcess(PCB *process)
    {
        std::lock_guard<std::mutex> guard(mutexForQueues);
        processes.push_back(process);
        // 初始状态为 READY 或 BLOCKED，根据 arrivalTime
        if (process->getArrivalTime() <= currentTime)
        {
            process->setCurrentState(PCB::READY);
            std::cout << "Current time: " << currentTime << " Process(" << process->getPid() << ") is in READY state." << std::endl;
            readyQueue.push(process);
        }
        else
        {
            // 使用 BLOCKED 表示进程尚未到达
            process->setCurrentState(PCB::BLOCKED);
            std::cout << "Current time: " << currentTime << " Process(" << process->getPid() << ") is in BLOCKED state (Not Arrived)." << std::endl;
            waitingQueue.push(process);
        }
    }

    void manageTimeAndSchedule(int selectedScheduleAlgorithm)
    {
        while (true)
        {
            // 检查是否所有进程都已终止
            if (areAllProcessesTerminated())
            {
                std::cout << "All processes have terminated at time " << currentTime << "." << std::endl;
                break;
            }

            // 检查并添加新到达的进程
            checkAndAddNewArrivedProcesses();

            // 恢复等待队列中的进程
            recoverWaitingProcesses();

            // 如果就绪队列为空，CPU 处于空闲状态
            if (readyQueue.empty())
            {
                std::cout << "Current time: " << currentTime << " CPU is idle." << std::endl;
                // 当 CPU 空闲时递增 currentTime
                currentTime++;
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 模拟时间流逝，减少等待时间
                continue;
            }

            // 根据调度算法选择并执行进程
            switch (selectedScheduleAlgorithm)
            {
            case 0: // 轮转调度
                RoundRobin_Schedule();
                break;
            case 1: // 先来先服务 (FCFS)
                FCFS_Schedule();
                break;
            case 2: // 最高优先级优先
                HighestPriorityFirst_Schedule();
                break;
            default:
                std::cerr << "Invalid scheduling algorithm selected." << std::endl;
                return;
            }
        }
    }

    void displayQueues() const
    {
        std::lock_guard<std::mutex> guard(mutexForQueues); // 确保队列操作的线程安全
        std::cout << "\nFinal Queue Status:" << std::endl;

        std::cout << "Ready Queue: ";
        std::queue<PCB *> tempReady = readyQueue;
        while (!tempReady.empty())
        {
            std::cout << tempReady.front()->getPid() << " ";
            tempReady.pop();
        }
        std::cout << std::endl;

        std::cout << "Terminated Queue: ";
        std::queue<PCB *> tempTerminated = terminatedQueue;
        while (!tempTerminated.empty())
        {
            std::cout << tempTerminated.front()->getPid() << " ";
            tempTerminated.pop();
        }
        std::cout << std::endl;
    }

private:
    int currentTime = 0;                                       // 当前系统时间
    int timeSlice;                                             // 轮转调度的时间片大小
    PCB *currentProcess;                                       // 当前执行的进程
    std::vector<PCB *> processes;                              // 所有进程
    std::queue<PCB *> readyQueue;                              // 就绪队列
    std::queue<PCB *> terminatedQueue;                         // 终止队列
    std::queue<PCB *> waitingQueue;                            // 等待队列（未到达或被阻塞的进程）
    mutable std::mutex mutexForQueues;                         // 队列操作的互斥锁
    std::chrono::steady_clock::time_point lastInstructionTime; // 记录上一次执行指令的时间

    bool inputAvailable;

    // 检查是否所有进程都已终止
    bool areAllProcessesTerminated() const
    {
        for (const auto &pcb : processes)
        {
            if (pcb->getCurrentState() != PCB::TERMINATED)
                return false;
        }
        return true;
    }

    // 检查并添加新到达的进程
    void checkAndAddNewArrivedProcesses()
    {
        std::lock_guard<std::mutex> guard(mutexForQueues);
        std::vector<PCB *> newlyArrivedProcesses;
        for (auto &pcb : processes)
        {
            // 如果进程处于 BLOCKED 并且 arrivalTime <= currentTime 且未被调度过
            // 这里假设 BLOCKED 状态既用于等待输入也用于未到达
            // 需要区分这两种情况，可以通过另一个标志或状态管理
            // 为简单起见，这里假设 BLOCKED 且 arrivalTime <= current_time 表示已到达
            if (pcb->getCurrentState() == PCB::BLOCKED && pcb->getArrivalTime() <= currentTime && pcb->getUsedRunTime() == 0 && pcb->programCounter == 0)
            {
                pcb->setCurrentState(PCB::READY);
                std::cout << "Current time: " << currentTime << " Process(" << pcb->getPid() << ") has arrived and is in READY state." << std::endl;
                readyQueue.push(pcb);
                newlyArrivedProcesses.push_back(pcb);
            }
        }

        // 从等待队列中移除已到达的进程
        for (auto pcb : newlyArrivedProcesses)
        {
            // 创建一个临时队列，排除已到达的进程
            std::queue<PCB *> tempQueue;
            while (!waitingQueue.empty())
            {
                PCB *proc = waitingQueue.front();
                waitingQueue.pop();
                if (proc != pcb)
                {
                    tempQueue.push(proc);
                }
            }
            waitingQueue = tempQueue;
        }
    }

    // 恢复等待队列中的进程（现已不使用多线程输入，保留以备未来扩展）
    void recoverWaitingProcesses()
    {
        // 保留此函数以备未来使用多线程输入时恢复等待的进程
    }

    // 获取进程当前等待的变量名
    std::string getCurrentReadVariable(PCB *process)
    {
        if (process->programCounter >= process->getCodeLength())
            return "";

        std::string instruction = code[process->getCodeStartIndex() + process->programCounter];
        std::istringstream iss(instruction);
        std::string cmd, var;
        iss >> cmd >> var;
        var = stripComma(var);
        return var;
    }

    // 轮转调度
    void RoundRobin_Schedule()
    {
        if (readyQueue.empty())
            return;

        {
            std::lock_guard<std::mutex> guard(mutexForQueues);
            currentProcess = readyQueue.front();
            readyQueue.pop();
        }

        currentProcess->setCurrentState(PCB::RUNNING);
        std::cout << "Current time: " << currentTime << " Process " << currentProcess->getPid() << " is RUNNING (Round Robin)." << std::endl;

        int executeTime = std::min(timeSlice, currentProcess->getTotalRunTime() - currentProcess->getUsedRunTime());

        for (int i = 0; i < executeTime; ++i)
        {
            // 执行指令或占用CPU时间
            executeInstruction(currentProcess);

            // 仅当进程处于 RUNNING 状态时递增 usedRunTime
            if (currentProcess->getCurrentState() == PCB::RUNNING)
            {
                currentProcess->updateUsedRunTime(1);
            }

            // 检查并添加新到达的进程
            checkAndAddNewArrivedProcesses();

            // 如果进程被设置为 BLOCKED 或 TERMINATED，提前退出
            if (currentProcess->getCurrentState() == PCB::BLOCKED ||
                currentProcess->getCurrentState() == PCB::TERMINATED)
            {
                break;
            }
        }

        // 调度结束后，根据进程状态决定下一步
        if (currentProcess->getUsedRunTime() >= currentProcess->getTotalRunTime())
        {
            currentProcess->setCurrentState(PCB::TERMINATED);
            std::cout << "Current time: " << currentTime << " Process " << currentProcess->getPid() << " has TERMINATED." << std::endl;
            {
                std::lock_guard<std::mutex> lock(mutexForQueues);
                terminatedQueue.push(currentProcess);
            }
            displayQueues();
        }
        else if (currentProcess->getCurrentState() == PCB::BLOCKED)
        {
            // 进程已进入 BLOCKED 状态，不重新加入就绪队列
            std::cout << "Current time: " << currentTime << " Process " << currentProcess->getPid() << " is BLOCKED." << std::endl;
        }
        else
        {
            // 进程时间片用完但未完成，重新加入就绪队列
            currentProcess->setCurrentState(PCB::READY);
            std::cout << "Current time: " << currentTime << " Process " << currentProcess->getPid() << " time slice expired, requeuing." << std::endl;
            {
                std::lock_guard<std::mutex> lock(mutexForQueues);
                readyQueue.push(currentProcess);
            }
        }

        currentProcess = nullptr;
    }

    // 先来先服务调度
    void FCFS_Schedule()
    {
        // 实现保持不变
    }

    // 最高优先级优先调度
    void HighestPriorityFirst_Schedule()
    {
        // 实现保持不变
    }

    void executeInstruction(PCB *process)
    {
        if (process->getUsedRunTime() < process->getTotalRunTime())
        {
            if (process->programCounter < process->getCodeLength())
            {
                std::string instruction = code[process->getCodeStartIndex() + process->programCounter];
                process->programCounter++;
                std::cout << "Current time: " << currentTime << " Process " << process->getPid()
                          << " is executing instruction: " << instruction << std::endl;
                process->programCounter++;
            }
            else
            {
                std::cout << "Current time: " << currentTime << " Process " << process->getPid()
                          << " is waiting for terminal..." << std::endl;
            }
            // 模拟指令执行时间消耗
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            currentTime++;
        }
        else
            std::cout << "a question!!!" << std::endl;
    }
};

#endif
