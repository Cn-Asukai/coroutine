#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include "task.hpp"
#include "task_info.hpp"
#include "thread_meta.hpp"
#include "user_data.hpp"
#include <coroutine>
#include <cstdint>
#include <iostream>
#include <liburing.h>
#include <queue>
#include <sys/eventfd.h>
#include <thread>

namespace coroutine {

class context {
public:
  context() {
    context_create_count++;
    io_uring_queue_init(64, &uring, 0);
    eventfd_ = ::eventfd(0, 0);
  }

  void start() {
    thread_ = std::jthread([this](std::stop_token st) {
      context_ready_count++;
      this_thread.ctx = this;
      if (context_create_count > 1) {

        // 监听eventfd
        listen_eventfd();
      }
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

  void co_spawn(task<> t) { co_spawn_auto(std::move(t)); }

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

      if (user_data < static_cast<uintptr_t>(reserved_user_data::none)) {
        handle_co_sapwn_event(user_data);
        continue;
      }

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
    }
    io_uring_cq_advance(&uring, count);
  }
  void join() { thread_.join(); }

private:
  void listen_eventfd() {
    io_uring_sqe *sqe = io_uring_get_sqe(&uring);
    io_uring_prep_read(sqe, eventfd_, &event_buf_, sizeof(event_buf_), 0);
    io_uring_sqe_set_data64(
        sqe, static_cast<uint64_t>(reserved_user_data::co_spawn_event));
  }

  void handle_co_sapwn_event(uintptr_t user_data) {
    // TODO 处理co_spawn事件
    // 将事件添加到队列中
    local_co_spawn_queue.swap(co_spawn_queue);

    while (!local_co_spawn_queue.empty()) {
      forward_task(local_co_spawn_queue.front());
      local_co_spawn_queue.pop();
    }

    listen_eventfd();
  }

  void co_spawn_eventfd(std::coroutine_handle<> h) {
    co_spawn_queue.emplace(h);
    eventfd_write(eventfd_, 1);
  }

  void co_spawn_auto(task<> t) {
    if (this_thread.ctx == this || context_ready_count == 0) {
      forward_task(t.get_handle());
    } else {
      co_spawn_eventfd(t.get_handle());
    }
  }

  inline static int context_create_count{0};
  inline static int context_ready_count{0};
  io_uring uring{};
  std::queue<std::coroutine_handle<>> handles_;
  std::jthread thread_;
  int eventfd_;
  eventfd_t event_buf_;
  std::queue<std::coroutine_handle<>> co_spawn_queue;
  std::queue<std::coroutine_handle<>> local_co_spawn_queue;
};

void co_spawn(task<> t);
} // namespace coroutine

#endif // CONTEXT_HPP
