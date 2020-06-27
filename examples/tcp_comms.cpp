#include <tcp_stream.h>
#include <thread>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <cstdint>
#include <iostream>
#include <cstring>

using namespace std;
using namespace umsg;

int  main(int argc, char** argv) {



    std::thread sendrecv([&](){
        sockaddr_in address = {0};
        address.sin_family = AF_INET;
        address.sin_port = ntohs(6900);
        inet_aton("127.0.0.1", &address.sin_addr);
        auto s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        tcp_stream out_stream(s);

        for(;;) {
            int r = connect(s, (const sockaddr*)&address, sizeof(sockaddr_in));
            if (r != 0) {
                std::cerr << "connection error:" << errno << std::endl;
            } else {
                break;
            }
        }
        const char* out = "testing\n";
        out_stream.send(std::vector<uint8_t>((const uint8_t*)out, (const uint8_t*) out + strlen(out)));
        out_stream.send(std::vector<uint8_t>{});

        for(;;)
        {
            auto buf = out_stream.recv();
            if (buf.size() == 0){
                break;
            }
            for(auto x:buf) {
                std::cout << (char) x << std::flush;
            }
        }
    });

    std::thread recvsend([&](){
        sockaddr_in address = {0};
        address.sin_family = AF_INET;
        address.sin_port = ntohs(6900);
        inet_aton("0.0.0.0", &address.sin_addr);
        auto l = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        int r = bind(l, (const sockaddr*) &address, sizeof(sockaddr_in));
        if (r != 0) {
            std::cerr << "bind error " << errno << std::endl;
            return;
        }

        r = listen(l, 10);
        if (r != 0) {
            std::cerr << "listen error " << errno << std::endl;
            return;
        }

        sockaddr_in incoming;
        socklen_t width = sizeof(incoming);
        unsigned long s = accept(l, (sockaddr*)&incoming, &width);

        if (s == 0) {
            std::cerr << "accept error " << errno << std::endl;
        }
        tcp_stream in_stream(s);
        for(;;)
        {
            auto buf = in_stream.recv();
            if (buf.size() == 0){
                break;
            }
            in_stream.send(buf);
        }
    });

    sendrecv.join();
    recvsend.join();

    return 0;
}