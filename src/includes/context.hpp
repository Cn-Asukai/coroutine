#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <algorithm>
#include <coroutine>
#include <functional>
#include <liburing.h>
#include <queue>
#include <thread>
#include <utility>
#include <span>
#include "task.hpp"
#include "thread_meta.hpp"

namespace coroutine {



class context {
public:
    context() {
        io_uring_queue_init(64, &uring, 0);
        this_thread.ctx = this;
    }

    void start() {
        while (true) {
            resume();

            io_uring_cqe* cqe;

            int wait_nr = 0;
            // TODO 设计一个队列，当队列为空，不等待
            io_uring_submit_and_wait(&uring, 1);

            unsigned head, count = 0;
            io_uring_for_each_cqe(&uring, head, cqe) {
                ++count;

                auto* task_i = static_cast<task_info *>(io_uring_cqe_get_data(cqe));

                task_i->result = cqe->res;
                handles_.emplace(task_i->handle);
            }
            io_uring_cq_advance(&uring, count);
        }
    }

    io_uring_sqe* get_sqe() {
        return io_uring_get_sqe(&uring);
    }

    void co_spawn(task t) {
        handles_.emplace(t.m_handle);
    }

    void resume() {
        for(int i = 0; i < handles_.size(); i++) {
            auto coroutine_handle = handles_.front();
            coroutine_handle.resume();
            handles_.pop();
        }
    }

private:
    io_uring uring{};
    std::queue<std::coroutine_handle<>> handles_;
};



} // namespace coroutine


#endif // CONTEXT_HPP
