#include <message_queue.h>
#include <iostream>

using namespace std;
using namespace umsg;

int main(int argc, char** argv) {

    message_queue<int> q;

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
    
    while(!q.empty())
    {
        int message = q.pop();
        std::cout << "read message: " << message << std::endl;
    }

    return 0;
}