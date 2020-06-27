#include <message_queue.h>
#include <iostream>

using namespace std;
using namespace umsg;

int main(int argc, char** argv) {

    message_queue<int> q;

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
    
    while(!q.empty())
    {
        int message = q.pop();
        std::cout << "read message: " << message << std::endl;
    }

    return 0;
}