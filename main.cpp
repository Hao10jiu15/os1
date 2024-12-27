// main.cpp
#include "all_cpu.h"
#include "allhead.h"
#include "all_pcb.h"

std::vector<std::string> code;

int main()
{
    // 设置信号处理
    signal(SIGINT, signalHandler);
    // 创建计时线程
    pthread_t timer;
    pthread_create(&timer, nullptr, countTime, nullptr);
    // 模拟程序运行，等待一段时间后停止计时线程
    // std::this_thread::sleep_for(std::chrono::seconds(10));

    std::vector<PCB *> processes;

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

    code.resize(ALL_MEMORY_SIZE, "");

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

    std::sort(processes.begin(), processes.end(), [](PCB *a, PCB *b)
              { return !(*a < *b); });

    for (auto &pcb : processes)
    {
        std::cout << "Process " << pcb->getPid() << " Priority: " << pcb->getPriority() << std::endl;
    }

    // 停止计时线程
    stopTimer = true;
    // 等待计时线程结束
    pthread_join(timer, nullptr);
    std::cout << "Main thread ends." << "CPU Time:" << CPU::currentTime << std::endl;
    return 0;
}
