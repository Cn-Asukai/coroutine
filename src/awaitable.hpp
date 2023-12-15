//
// Created by Asukai on 2023/11/21.
//

#ifndef AWAITABLE_HPP
#define AWAITABLE_HPP

#include "context.hpp"
#include "task_info.hpp"
#include "thread_meta.hpp"
#include "time_cast.hpp"
#include "user_data.hpp"
#include <chrono>
#include <cstdint>
#include <liburing.h>
#include <span>

namespace coroutine {
struct awaitable {
  explicit awaitable() : sqe(this_thread.ctx->get_sqe()) {
    io_uring_sqe_set_data(
        sqe, reinterpret_cast<void *>(
                 ti.as_user_data() |
                 static_cast<uint64_t>(user_data_type::task_info_ptr)));
  }

  static bool await_ready() { return false; }

  void await_suspend(std::coroutine_handle<> handle) {
    ti.handle = handle;
    io_uring_sqe_set_data(sqe, &ti);
  }

  int await_resume() const { return ti.res; }

  io_uring_sqe *sqe;
  task_info ti{};
};

namespace detail {
struct co_accept : awaitable {
  co_accept(int fd, sockaddr *addr, socklen_t *addrlen) {
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

struct co_close : awaitable {
  explicit co_close(int fd) { io_uring_prep_close(sqe, fd); }
};

struct co_read : awaitable {
  explicit co_read(int fd, std::span<char> buf, size_t len) {
    io_uring_prep_read(sqe, fd, buf.data(), len, 0);
  }
};

struct co_timeout_base : awaitable {
protected:
  __kernel_timespec ts{};

public:
  template <typename Rep, typename Period>
  explicit co_timeout_base(std::chrono::duration<Rep, Period> duration) {
    set_ts(duration);
  }

  template <typename Rep, typename Period>
  void set_ts(std::chrono::duration<Rep, Period> duration) {
    ts = to_kernel_timespec(duration);
  }
};

struct co_timeout : co_timeout_base {
  template <typename Rep, typename Period>
  explicit co_timeout(std::chrono::duration<Rep, Period> duration)
      : co_timeout_base(duration) {
    io_uring_prep_timeout(sqe, &ts, 0, 0);
  }
};
} // namespace detail
} // namespace coroutine

#endif // AWAITABLE_HPP
