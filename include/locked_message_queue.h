
#include <queue>
#include <deque>
#include <type_traits>
#include <mutex>
#include <discard_message.h>

namespace umsg {
    template<
        typename TX, //could, for example be a const ref
        typename TN = discard_message<void>, //notification handler type
        typename T =  typename std::decay<TX>::type,
        typename TQ = std::queue<T>,
        typename TM = std::mutex>
    class locked_message_queue
    {
        typedef locked_message_queue<TX, TN, T, TQ, TM> class_t;
        TQ q_;
        TN n_;
        mutable TM mut_;
    public:
        locked_message_queue(){ //TN must have a default constructor
        }
        locked_message_queue(TN n):n_(n){
        }
        locked_message_queue(const class_t& cq) {
            auto& q = const_cast<class_t&>(cq);
            std::lock_guard<TM> lock(q.mut_);
            q_ = q.q_;
            n_ = q.n_;
        }
        locked_message_queue(class_t&& q) {
            std::lock_guard<TM> lock(q.mut_);
            q_ = q.q_;
            n_ = q.n_;
        }
        void operator=(const class_t& cq) {
            auto& q = const_cast<class_t&>(cq);
            std::scoped_lock lock(mut_, q.mut_);
            q_ = q.q_;
            n_ = q.n_;
        }
        void operator=(class_t&& q) {
            std::scoped_lock lock(mut_, q.mut_);
            q_ = q.q_; // todo: should this be a swap?
            n_ = q.n_;
            q.q_ = TQ();
        }

        //TODO: conversion constructors for compatible queue types

        ~locked_message_queue() = default; //I think this is alright

        void msg(TX t) {
            std::lock_guard<TM> lock(mut_);
            q_.push(t);
            n_.msg(); //notify listener
        }
        auto size() const {
            std::lock_guard<TM> lock(mut_);
            return q_.size();
        }
        bool empty() const {
            std::lock_guard<TM> lock(mut_);
            return q_.empty();
        }
        T& front() const {
            std::lock_guard<TM> lock(mut_);
            if (q_.empty()) {
                std::logic_error("empty");
            }
            return q_.front();
        }
        T pop() {
            std::lock_guard<TM> lock(mut_);
            if (q_.empty()) {
                std::logic_error("empty");
            }
            auto result = q_.front();
            q_.pop();
            return result;
        }

        // DEADLOCK WARNING: take care using this method,
        // as circular locks may occur. Using a recursive_mutex
        // may help here but may not be enough in the case
        // of circular locks between several threads
        // also note, if a receiver fires an exception here the
        // message will not be dequeued
        template<typename TR>
        void recv(TR& tr) {
            std::lock_guard<TM> lock(mut_);
            if (!q_.empty()) {
                tr.msg(q_.front());
                q_.pop();
            }
        }
    };
}
