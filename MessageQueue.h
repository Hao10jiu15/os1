#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

class MessageQueue
{
private:
    std::queue<std::string> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    // 发送消息
    void sendMessage(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(message);
        cv.notify_one(); // 通知等待的线程有新消息
    }

    // 接收消息
    std::string receiveMessage()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]()
                { return !queue.empty(); }); // 如果队列为空，等待新消息
        std::string message = queue.front();
        queue.pop();
        return message;
    }
};
