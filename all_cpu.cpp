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
#include <atomic>
#include <csignal>

// code 是全局变量，用于存储所有进程的指令
extern std::vector<std::string> code;

// 用于控制计时线程的标志
std::atomic<bool> stopTimer(false);
std::mutex timeMutex; // 用于同步访问 currentTime

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
    static int currentTime; // 当前系统时间

    CPU(int _timeSlice)
        : timeSlice(_timeSlice),
          currentProcess(nullptr),
          inputAvailable(false),
          lastInstructionTime(std::chrono::steady_clock::now()) // 初始化上一次指令执行时间为当前时间
    {
    }

    static void incrementTime()
    {
        // 增加 currentTime 时需要加锁以避免竞态条件
        std::lock_guard<std::mutex> guard(timeMutex);
        currentTime++;
    }

private:
    int timeSlice;                                             // 轮转调度的时间片大小
    PCB *currentProcess;                                       // 当前执行的进程
    std::vector<PCB *> processes;                              // 所有进程
    std::queue<PCB *> readyQueue;                              // 就绪队列
    std::queue<PCB *> terminatedQueue;                         // 终止队列
    std::queue<PCB *> blockedQueue;                            // 阻塞队列
    mutable std::mutex mutexForQueues;                         // 队列操作的互斥锁
    std::chrono::steady_clock::time_point lastInstructionTime; // 记录上一次执行指令的时间

    bool inputAvailable;
};

int CPU::currentTime = 0;

#endif

void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        std::cout << "Received SIGINT, stopping the timer..." << std::endl;
        stopTimer = true; // 设置标志，通知计时线程停止
    }
}

void *countTime(void *arg)
{
    auto start = std::chrono::steady_clock::now();

    while (!stopTimer)
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - start;

        // 使用锁来保护 currentTime 访问
        {
            std::lock_guard<std::mutex> guard(timeMutex);
            std::cout << "Time elapsed: " << elapsed.count() << " seconds, CPU currentTime: " << CPU::currentTime << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2)); // 每2秒更新一次
    }

    std::cout << "Timer stopped." << std::endl;
    return nullptr;
}

int main()
{
    // 设置信号处理
    signal(SIGINT, signalHandler);

    // 创建计时线程
    pthread_t timer;
    pthread_create(&timer, nullptr, countTime, nullptr);

    // 模拟程序运行，等待一段时间后停止计时线程
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // 停止计时线程
    stopTimer = true;

    // 等待计时线程结束
    pthread_join(timer, nullptr);

    std::cout << "Main thread ends." << std::endl;

    return 0;
}
