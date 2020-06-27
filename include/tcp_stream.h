#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

namespace umsg {
    
    class tcp_stream {
    public:
        typedef int socket_t;

        tcp_stream();
        tcp_stream(socket_t s);
        tcp_stream(const tcp_stream& s);
        tcp_stream(tcp_stream&& s);
        void operator=(const tcp_stream& s);
        void operator=(tcp_stream&& s);
        ~tcp_stream();
        void msg(const std::vector<unsigned char>& packet);
        bool ready();
        std::vector<unsigned char> recv();

        template<typename T>
        void recv(T& recvr) {
            auto v = recv();
            recvr.msg(v);
        }
    private:
        mutable socket_t socket_;
    };
}