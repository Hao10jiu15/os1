#ifndef PCB_H
#define PCB_H

#include "allhead.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>

// 前向声明，避免循环包含问题，如果在完整代码中需要包含相应头文件则要正确包含进来
class CPU;

struct Context
{
    std::unordered_map<std::string, int> registers; // 模拟寄存器
};

struct StackFrame
{
    Context context;
    int programCounter;
};

class Stack
{
public:
    Stack() {}

    void push(const StackFrame &frame)
    {
        stack.push_back(frame);
    }

    StackFrame pop()
    {
        if (!stack.empty())
        {
            StackFrame topFrame = stack.back();
            stack.pop_back();
            return topFrame;
        }

        return StackFrame();
    }

    bool isEmpty() const
    {
        return stack.empty();
    }

private:
    std::vector<StackFrame> stack;
};

class PCB
{
public:
    // 枚举类型 State 在公共部分声明
    enum State
    {
        READY,
        RUNNING,
        BLOCKED,
        TERMINATED
    };

    PCB(long long int _pid = 0, int _priority = 0, int _arrivalTime = 0, int _totalRunTime = 0, State _currentState = READY, std::shared_ptr<PCB> _parent = nullptr, int _memoryUsage = 0)
        : pid(_pid),
          priority(_priority),
          arrivalTime(_arrivalTime),
          totalRunTime(_totalRunTime),
          usedRunTime(0),
          currentState(_currentState),
          parent(_parent),
          memoryUsage(_memoryUsage),
          programCounter(0),
          usedTimeSlice(0),
          remainingTimeSlice(0)
    {
        numOfpro = proNum++;
    }

    // Getters 和 Setters
    long long int getPid() const { return pid; }
    int getPriority() const { return priority; }
    void setPriority(int p) { priority = p; }
    State getCurrentState() const { return currentState; }
    void setCurrentState(State newState) { currentState = newState; }
    int getTotalRunTime() const { return totalRunTime; }
    int getUsedRunTime() const { return usedRunTime; }
    void updateUsedRunTime(int additionalTime) { usedRunTime += additionalTime; }

    std::shared_ptr<PCB> getParent() const { return parent; }
    void setParent(std::shared_ptr<PCB> newParent) { parent = newParent; }

    int getMemoryUsage() const { return memoryUsage; }
    void updateMemoryUsage(int additionalMemory) { memoryUsage += additionalMemory; }

    int getUsedTimeSlice() const { return usedTimeSlice; }
    void updateUsedTimeSlice() { usedTimeSlice++; }

    int getRemainingTimeSlice() const { return remainingTimeSlice; }
    void setRemainingTimeSlice(int newRemainingTimeSlice) { remainingTimeSlice = newRemainingTimeSlice; }

    void setCodeInfo(int startIndex, int length)
    {
        codeStartIndex = startIndex;
        codeLength = length;
    }

    int getCodeStartIndex() const { return codeStartIndex; }
    int getCodeLength() const { return codeLength; }

    int getArrivalTime() const { return arrivalTime; } // 获取 arrivalTime 的 Getter

    // 重载小于运算符用于优先级比较
    bool operator<(const PCB &other) const
    {
        return this->priority < other.priority; // 优先级值越高，优先级越高
    }

    // 保存当前进程上下文到PCB中的Context结构体
    void saveContext()
    {
        // 使用嵌入汇编将通用寄存器的值保存到context.registers中
        __asm__ volatile(
            "movl %%eax, %0\n"
            "movl %%ebx, %1\n"
            "movl %%ecx, %2\n"
            "movl %%edx, %3\n"
            : "=m"(context.registers["eax"]), "=m"(context.registers["ebx"]), "=m"(context.registers["ecx"]), "=m"(context.registers["edx"])
            :
            : "eax", "ebx", "ecx", "edx");
    }

    // 从PCB中的Context结构体恢复上下文到相应运行环境
    void restoreContext()
    {
        // 使用嵌入汇编从context.registers中恢复值到通用寄存器
        __asm__ volatile(
            "movl %0, %%eax\n"
            "movl %1, %%ebx\n"
            "movl %2, %%ecx\n"
            "movl %3, %%edx\n"
            :
            : "m"(context.registers["eax"]), "m"(context.registers["ebx"]), "m"(context.registers["ecx"]), "m"(context.registers["edx"])
            : "eax", "ebx", "ecx", "edx");
    }

    int numOfpro;

    // 公共成员
    State currentState;

    std::shared_ptr<PCB> parent; // 使用 shared_ptr 管理父进程
    Context context;
    int memoryUsage;
    int codeStartIndex;
    int codeLength;
    int usedTimeSlice;
    int remainingTimeSlice;

    Stack stack;

    // 程序计数器
    int programCounter;

private:
    long long int pid;
    int priority;
    int arrivalTime;
    int totalRunTime;
    int usedRunTime;
    static int proNum;
};

// 初始化静态成员
int PCB::proNum = 0;

#endif