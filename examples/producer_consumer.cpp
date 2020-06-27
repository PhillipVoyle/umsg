
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

    void msg() {
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

    q.msg(1);
    q.msg(2);
    q.msg(3);
    q.msg(4);
    q.msg(5);
    q.msg(6);
    q.msg(7);
    q.msg(8);
    q.msg(9);
    q.msg(10);
    
    q.msg(0); //done

    t.join();
    return 0;
}
