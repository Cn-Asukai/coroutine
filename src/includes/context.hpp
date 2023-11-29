#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include "task.hpp"
#include "task_info.hpp"
#include "thread_meta.hpp"
#include "user_data_type.hpp"
#include <algorithm>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <iostream>
#include <liburing.h>
#include <queue>
#include <span>
#include <thread>
#include <utility>
namespace coroutine {

class context {
public:
  context() {
    io_uring_queue_init(64, &uring, 0);
  }

  void start() {
    thread_ = std::jthread([this](std::stop_token st) {

    this_thread.ctx = this;
      while (!st.stop_requested()) {
        do_work_part();

        int wait_nr = handles_.empty();
        // TODO 设计一个队列，当队列为空，不等待
        io_uring_submit_and_wait(&uring, wait_nr);

        do_completion_part();
      }
    });
  }

  io_uring_sqe *get_sqe() { return io_uring_get_sqe(&uring); }

  void forward_task(std::coroutine_handle<> h) { handles_.emplace(h); }

  void co_spawn(task<> t) { handles_.emplace(t.get_handle()); }

  void do_work_part() {
    int n = static_cast<int>(handles_.size());
    for (int i = 0; i < n; i++) {
      std::coroutine_handle<> coroutine_handle = handles_.front();
      handles_.pop();
      coroutine_handle.resume();
    }
  }

  void do_completion_part() {

    io_uring_cqe *cqe;
    unsigned head, count = 0;
    io_uring_for_each_cqe(&uring, head, cqe) {
      ++count;

      auto user_data = reinterpret_cast<uintptr_t>(io_uring_cqe_get_data(cqe));
      auto type = user_data_type(user_data & 0b111);
      user_data &= task_info_ptr_mask;

      switch (type) {
      case user_data_type::task_info_ptr: {
        auto ti = reinterpret_cast<task_info *>(user_data);
        std::cout << "ti->res:" << cqe->res << std::endl;
        ti->res = cqe->res;
        forward_task(ti->handle);
        break;
      }
      }
      io_uring_cq_advance(&uring, count);
    }
  }
  void join() { thread_.join(); }

private:
  io_uring uring{};
  std::queue<std::coroutine_handle<>> handles_;
  std::jthread thread_;
};

void co_spawn(task<> t);
} // namespace coroutine

#endif // CONTEXT_HPP
