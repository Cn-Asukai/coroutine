#ifndef TASK_HPP
#define TASK_HPP

#include <coroutine>
#include <liburing.h>
#include <sys/types.h>

namespace coroutine {

struct task_info {
  std::coroutine_handle<> handle{};
  int32_t result{};
};

class task {
  struct awaiter_base {

  };

public:
  struct promise_type {
    using handle = std::coroutine_handle<promise_type>;
    task get_return_object() { return task{handle::from_promise(*this)}; }

    std::suspend_always initial_suspend() noexcept { return {}; }

    std::suspend_never final_suspend() noexcept { return {}; }

    void return_void() noexcept {}

    void unhandled_exception() {}

    io_uring *uring;
  };

  explicit task(const std::coroutine_handle<promise_type> handle) : m_handle(handle) {}

  task(task &&rhs) noexcept : m_handle(rhs.m_handle) { rhs.m_handle = {}; }

  auto operator co_await() {
      struct awaiter:awaiter_base {
        bool await_ready() {
          // TODO
            return false;
        }
        void await_suspend() {

        }
      };
    return awaiter{};
  }

  std::coroutine_handle<promise_type> m_handle{};
};
} // namespace coroutine


#endif // TASK_HPP