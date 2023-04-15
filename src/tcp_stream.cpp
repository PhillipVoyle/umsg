
#include <tcp_stream.h>
#include <unistd.h>
namespace umsg {

    tcp_stream::tcp_stream() {
        socket_ = 0;
    }

    tcp_stream::tcp_stream(tcp_stream::socket_t s):socket_(s) {
    }

    tcp_stream::tcp_stream(const tcp_stream& s) {
        socket_ = s.socket_;
        s.socket_ = 0;
    }

    tcp_stream::tcp_stream(tcp_stream&& s) {
        socket_ = s.socket_;
        s.socket_ = 0;
    }

    void tcp_stream::internal_close()
    {
#ifdef __WIN32            
        closesocket(socket_);
#else
        close(socket_);
#endif
    }

    void tcp_stream::operator=(const tcp_stream& s) {
        if(socket_ != 0 && socket_ != s.socket_) {
            close(socket_);
        }
        socket_ = s.socket_;
        s.socket_ = 0;
    }

    void tcp_stream::operator=(tcp_stream&& s) {
        if(socket_ != 0 && socket_ != s.socket_) {
            close(socket_);
        }
        socket_ = s.socket_;
        s.socket_ = 0;
    }


    tcp_stream::~tcp_stream() {
        internal_close();
    }

    void tcp_stream::send(const std::vector<unsigned char>& packet) {
        if (packet.size() == 0) {
            shutdown(socket_, 1);
        } else  {
            ::send(socket_, packet.data(), packet.size(), 0);
        }
    }

    bool tcp_stream::ready() {
        if (socket != 0)
        {
            fd_set rfds;
            timeval tv = {0};

            FD_ZERO(&rfds);
            FD_SET(socket_, &rfds);
            auto recVal = select(socket_ + 1, &rfds, NULL, NULL, &tv);

            if(recVal > 0) {
                return FD_ISSET(socket_ , &rfds);
            }
        }
        return false;
    }


    std::vector<unsigned char> tcp_stream::recv() {
        std::vector<unsigned char> buf;
        if (socket_ == 0) {
            return buf;
        }

        buf.resize(1024, '\0');
        int recv = ::recv(socket_, buf.data(), 1024, 0);
        if (recv < 0) {
            buf.resize(0);
        } else {
            buf.resize(recv);
        }
        return buf;
    }
}