//
// Created by Asukai on 2023/11/21.
//

#ifndef AWAITABLE_HPP
#define AWAITABLE_HPP


#include "context.hpp"
#include "task.hpp"
#include "thread_meta.hpp"

namespace coroutine {
struct awaitable {
    explicit awaitable(): sqe(this_thread.ctx->get_sqe()) {
    }


    bool await_ready() {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle) {
        ti.handle = handle;
        io_uring_sqe_set_data(sqe, &ti);
    }

    int await_resume() {
        return ti.result;
    }


    io_uring_sqe* sqe;
    task_info ti{};
};

namespace detail {
    struct co_accept : awaitable {
        co_accept(int fd, sockaddr* addr, socklen_t* addrlen) {
            io_uring_prep_accept(sqe, fd, addr, addrlen, 0);
        }
    };

    struct co_recv : awaitable {
        co_recv(int fd, std::span<char> buf) {
            io_uring_prep_recv(sqe, fd, buf.data(), buf.size(), 0);
        }
    };

    struct co_send : awaitable {
        co_send(int fd, std::span<char> buf, size_t len) {
            io_uring_prep_send(sqe, fd, buf.data(), len, 0);
        }
    };
}
}

#endif //AWAITABLE_HPP
