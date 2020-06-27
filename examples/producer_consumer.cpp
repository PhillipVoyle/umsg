
#include <thread>
#include <locked_message_queue.h>
#include <condition_variable>
#include <thread>
#include <iostream>

using namespace std;
using namespace umsg;

struct on_notify
{
    std::condition_variable cv_;

    void send() {
        cv_.notify_one();
    }
};

int main(int argc, char** argv) {

    std::mutex mut;
    on_notify n;
    locked_message_queue<int, on_notify&> q{n};
    std::thread t([&]() {
        std::unique_lock<std::mutex> lock;
        for(;;)
        {
            n.cv_.wait(lock, [&]() {return !q.empty();});
            int message = q.pop();
            std::cout << "read message: " << message << std::endl;
            if (message == 0) {
                break;
            }
        }
    });

    q.send(1);
    q.send(2);
    q.send(3);
    q.send(4);
    q.send(5);
    q.send(6);
    q.send(7);
    q.send(8);
    q.send(9);
    q.send(10);
    
    q.send(0); //done

    t.join();
    return 0;
}
