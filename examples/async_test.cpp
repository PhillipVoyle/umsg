#include <memory>
#include <cstdint>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <iostream>
#include <deque>
#include <tuple>
#include <chrono>
#include <algorithm>

using namespace std::chrono;

struct address {
    sockaddr* addr;
    int size;
};

struct io_buffer {
    uint8_t* buffer;
    int size_in;
    int size_out;
    int error;
    void* userdata;
};

class posix_select_thread {
    std::atomic_bool exit_loop_;
    std::atomic_bool interrupted_;
    std::thread thread_;

    struct file_info {
        int fileno;
        std::function<void()> on_read_ready;
        std::function<void()> on_write_ready;

        std::deque<std::tuple<io_buffer*, std::function<void(io_buffer*)>>> read_queue;
        std::deque<std::tuple<io_buffer*, std::function<void(io_buffer*)>>> write_queue;
    };
    std::vector<std::function<void()>> actions_;
    std::mutex mut_;

    std::vector<file_info> files_;
    int pipe_[2];

    void interrupt() {
        if(!interrupted_) {
            interrupted_ = true;
            char c = 'c';
            write(pipe_[1], &c, sizeof(c));
        }
    }

    void run_queued_actions() {
        // atomically retreive actions
        std::unique_lock<std::mutex> lock(mut_);
        std::vector<std::function<void()>> actions;
        std::swap(actions, actions_);
        lock.unlock();

        for(auto action: actions) {
            action();
        }
    }

    void loop_body() {

        // clear file descriptor set
        fd_set read_fds;
        fd_set write_fds;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);


        int nfds = 0;

        // build set
        FD_SET(pipe_[0], &read_fds);
        nfds = std::max({nfds, pipe_[0] + 1});

        // build timeout
        struct timeval *ptimeout = nullptr;

        int select_result = select(nfds, &read_fds, &write_fds, nullptr, ptimeout);
        if (select_result < 0) {

            std::cerr << "select error" << std::endl;

        } else {
            if (select_result == 0) {
                //timeout
            }
            if (FD_ISSET(pipe_[0], &read_fds)) {
                char c = '\0';
                interrupted_ = false;
                read(pipe_[0], &c, sizeof(c));
                std::cout << "interrupted" << std::endl;
            }

            std::vector<std::function<void()>> callbacks; 
            for(auto& info: files_) {
                int fileno = info.fileno;
                if (FD_ISSET(info.fileno, &read_fds)) {
                    if (!info.read_queue.empty()) {
                        auto& front = info.read_queue.front();

                        int fileno = info.fileno;
                        io_buffer* buf = std::get<0>(front);
                        callbacks.push_back([fileno, buf]() {
                            buf->size_out = read(fileno, buf->buffer, buf->size_in);
                            buf->error = errno;
                        });

                        info.read_queue.pop_front();
                    } else if (info.on_read_ready) {
                        auto read_callback = info.on_read_ready;
                        callbacks.push_back([read_callback](){
                            read_callback();
                        });
                        info.on_read_ready = nullptr;
                    }
                }
                if (FD_ISSET(info.fileno, &write_fds)) {
                    if (!info.write_queue.empty()) {

                        auto& front = info.write_queue.front();

                        io_buffer* buf = std::get<0>(front);
                        callbacks.push_back([fileno, buf]() {
                            buf->size_out = write(fileno, buf->buffer, buf->size_in);
                            buf->error = errno;
                        });

                        info.write_queue.pop_front();
                    } else if (info.on_write_ready) {
                        auto write_callback = info.on_write_ready;
                        callbacks.push_back([write_callback](){
                            write_callback();
                        });
                        info.on_write_ready = nullptr;
                    }
                }            
            }

            for(int a= 0; a < callbacks.size(); a++) {
                callbacks[a]();
            }            
        }

        run_queued_actions();
    }

    void run() {
        for(;;) {
            if (exit_loop_)
                return;
            loop_body();
        }
        run_queued_actions();
    }

    void set_timer(std::chrono::high_resolution_clock::time_point time, std::function<void()> on_timeout) {

    }

    void start() {
        thread_ = std::thread([this]() {
            run();
        });
    }

public:
    posix_select_thread() {
        pipe(pipe_);
        exit_loop_ = false;
        interrupted_ = false;
        start();
    }

    ~posix_select_thread() { //don't call this from running thread - will deadlock
        set_stop_flag();
        thread_.join();
    }

    void set_stop_flag() {
        if (!exit_loop_.exchange(true)) { // just once
            interrupt();
        }
    }

    bool queue_action(std::function<void()> action) {
        std::unique_lock<std::mutex> lock(mut_);
        if (exit_loop_) {
            return false;
        } else {
            actions_.push_back(action);
            lock.unlock();
            interrupt();
            return true;
        }
    }
};

int main(int argc, char** argv) {
    posix_select_thread thread;
    thread.queue_action([&thread](){
        std::cout << "test" << std::endl;
        thread.queue_action([](){ std::cout << "another action" << std::endl;});
        thread.queue_action([&thread](){ thread.set_stop_flag(); });
    });
    //std::this_thread::sleep_for(100ms);
    return 0;
}