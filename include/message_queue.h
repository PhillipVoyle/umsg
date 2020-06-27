#pragma once

#include <queue>
#include <deque>
#include <type_traits>
#include <stdexcept>

namespace umsg {
    template<
        typename TX, //could, for example be a const ref
        typename T = typename std::decay<TX>::type,
        typename TQ = std::queue<T>>
    class message_queue
    {
        TQ q_;
    public:
        void send(TX t) {
            q_.push(t);
        }
        auto size() {
            return q_.size();
        }
        bool empty() {
            return q_.empty();
        }
        T& front() {
            if (q_.empty()) {
                std::logic_error("empty");
            }
            return q_.front();
        }
        T pop() {
            if (q_.empty()) {
                std::logic_error("empty");
            }

            auto result = q_.front();
            q_.pop();
            return result;
        }
        template<typename TR>
        void recv(TR& tr) {
            if (!q_.empty()) {
                tr.send(q_.front());
                q_.pop();
            }
        }
    };
};
