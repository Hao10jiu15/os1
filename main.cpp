// main.cpp
#include "pcb.h"
#include "cpu.h"
#include "allhead.h" // 确保包含 allhead.h
#include <thread>
#include <atomic>
#include <iostream>

// 定义全局 code 向量
std::vector<std::string> code;

int main()
{
    int numProcesses = 3; // 三个进程

    CPU cpu(3); // Round Robin 的时间片为 3

    std::vector<PCB *> processes;

    // 定义每个进程的指令集（将原始代码改成类似指令集的格式）
    std::vector<std::string> instructions_p1 = {
        "#include <iostream>",
        "int main() {",
        "    std::cout << \"Hello World!\" << std::endl;",
        "    return 0;",
        "}"};

    std::vector<std::string> instructions_p2 = {
        "#include <iostream>",
        "int main() {",
        "    int a = 0, b = 0;",
        "    std::cin >> a;",
        "    std::cout << a + b << std::endl;",
        "    return 0;",
        "} "};

    std::vector<std::string> instructions_p3 = {
        "#include <iostream>",
        "int main() {",
        "    int a = 0, b = 0;",
        "    std::cin >> a >> b;",
        "    std::cout << a + b << std::endl;",
        "    return 0;",
        "} "};

    std::vector<std::string> instructions_p4 = {
        "#include <iostream>",
        "using namespace std;",
        "int main() {",
        "    char a = 'a';",
        "    if (a == 'a')",
        "        printf(\"%c\", a);",
        "    else",
        "        printf(\"not a\");",
        "    return 0;",
        "} "};

    std::vector<std::string> instructions_p5 = {
        "#include <iostream>",
        "using namespace std;",
        "int main() {",
        "    int a = 1, b = 2;",
        "    cout << a * b << endl;",
        "    return 0;",
        "} "};

    // 创建进程控制块（PCB）实例
    PCB *pcb1 = new PCB(1, 30, 0, instructions_p1.size() + 4); // Process 1
    PCB *pcb2 = new PCB(2, 20, 2, instructions_p2.size() + 2); // Process 2
    PCB *pcb3 = new PCB(3, 40, 4, instructions_p3.size() + 1); // Process 3
    PCB *pcb4 = new PCB(4, 25, 3, instructions_p4.size() + 3); // Process 4
    PCB *pcb5 = new PCB(5, 35, 1, instructions_p5.size() + 1); // Process 5

    processes.push_back(pcb1);
    processes.push_back(pcb2);
    processes.push_back(pcb3);
    processes.push_back(pcb4);
    processes.push_back(pcb5);

    // 初始化全局 code 向量以容纳所有指令
    code.resize(ALL_MEMORY_SIZE, ""); // 初始化为空字符串

    // Function to load instructions into code array
    auto loadInstructions = [&](const std::vector<std::string> &instructions, PCB *pcb)
    {
        int offset = pcb->numOfpro * 256; // 假设每个进程最多256条指令
        for (size_t i = 0; i < instructions.size() && (offset + i) < ALL_MEMORY_SIZE; ++i)
        {
            code[offset + i] = instructions[i];
        }
        pcb->setCodeInfo(offset, instructions.size());
    };

    // 加载指令到 code 中
    loadInstructions(instructions_p1, pcb1);
    loadInstructions(instructions_p2, pcb2);
    loadInstructions(instructions_p3, pcb3);
    loadInstructions(instructions_p3, pcb4);
    loadInstructions(instructions_p3, pcb5);

    // 将进程添加到 CPU
    for (auto &pcb : processes)
    {
        cpu.addProcess(pcb);
    }

    // 选择调度算法
    int selectedScheduleAlgorithm = 0; // 0: Round Robin, 1: FCFS, 2: Highest Priority First

    std::cout << "Starting CPU scheduling, using algorithm: ";
    switch (selectedScheduleAlgorithm)
    {
    case 0:
        std::cout << "Round Robin" << std::endl;
        break;
    case 1:
        std::cout << "First Come First Serve (FCFS)" << std::endl;
        break;
    case 2:
        std::cout << "Highest Priority First" << std::endl;
        break;
    default:
        std::cout << "Unknown" << std::endl;
        break;
    }

    // 启动 CPU 调度
    cpu.manageTimeAndSchedule(selectedScheduleAlgorithm);

    // 显示最终队列状态
    cpu.displayQueues();

    // 清理内存
    for (auto &pcb : processes)
    {
        delete pcb;
    }

    return 0;
}
