#ifndef all_pcb_h
#define all_pcb_h

#include "allhead.h"

typedef struct Context
{
    std::unordered_map<std::string, int> registers; //
} Context;

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
    enum State
    {
        READY,
        RUNNING,
        BLOCKED,
        TERMINATED
    };

    PCB(int _pid = 0, int _priority = 0, int _arrivalTime = 0, int _totalRunTime = 0, State _currentState = BLOCKED)
        : pid(_pid),
          priority(_priority),
          arrivalTime(_arrivalTime),
          totalRunTime(_totalRunTime),
          usedRunTime(0),
          currentState(_currentState),
          programCounter(0),
          usedTimeSlice(0),
          remainingTimeSlice(0)
    {
        numOfpro = proNum++;
    }

    int getPid() const { return pid; }
    int getPriority() const { return priority; }
    void setPriority(int p) { priority = p; }
    State getCurrentState() const { return currentState; }
    int getUsedRunTime() const { return usedRunTime; }
    void updateUsedRunTime(int additionalTime) { usedRunTime += additionalTime; }

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

    int getArrivalTime() const { return arrivalTime; }

    // 重载小于运算符用于优先级比较
    bool operator<(const PCB &other) const
    {
        return !(this->priority < other.priority);
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
    void setCurrentState(State newState) { currentState = newState; }
    int getTotalRunTime() const { return totalRunTime; }

    int numOfpro;

    // 公共成员
    State currentState;
    Context context;
    int codeStartIndex;
    int codeLength;
    int usedTimeSlice;
    int remainingTimeSlice;

    Stack stack;

    int programCounter;

private:
    int pid;
    int priority;
    int arrivalTime;
    int totalRunTime;
    int usedRunTime;
    static int proNum;
};

int PCB::proNum = 0;

#endif
