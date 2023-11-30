#ifndef TASK_HPP
#define TASK_HPP

#include <cassert>
#include <coroutine>
#include <exception>
#include <memory>
#include <sys/types.h>

namespace coroutine {
template <typename T> class task;

namespace detail {
template <typename T> class task_promise_base;

/**
 * @brief 如果当前task完成，那么唤醒调用方
 * 即 resume 父协程
 */
template <typename T> struct task_final_awaiter {
  bool await_ready() { return false; }

  // 在final_suspend中，协程并不会挂起
  // 如果co_await final_suspend 如果挂起了，那么协程将自动销毁
  // 如果没有挂起，那么协程需要手动调用task.destory()销毁
  template <typename Promise>
  std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> h) {
    return h.promise().parent_coro;
  }

  // 这里不会被调用，因为协程不会被恢复
  void await_resume() {}
};

template <> struct task_final_awaiter<void> {
  bool await_ready() noexcept { return false; }

  // 在final_suspend中，协程并不会挂起
  // 如果co_await final_suspend 如果挂起了，那么协程将自动销毁
  // 如果没有挂起，那么协程需要手动调用task.destory()销毁
  template <std::derived_from<task_promise_base<void>> Promise>
  std::coroutine_handle<>
  await_suspend(std::coroutine_handle<Promise> h) noexcept {

    auto &promise = h.promise();
    auto parent_coro = promise.parent_coro;

    // 不需要返回值，所有协程可以直接销毁
    // h.destroy();

    return parent_coro;
  }

  // 这里不会被调用，因为协程不会被恢复
  void await_resume() noexcept {}
};

template <typename T> struct task_promise_base {
  friend struct task_final_awaiter<T>;
  task_promise_base() = default;

  std::suspend_always initial_suspend() { return {}; }
  task_final_awaiter<T> final_suspend() noexcept { return {}; }

  void set_parent(std::coroutine_handle<> continuation) {
    parent_coro = continuation;
  }

  task_promise_base(const task_promise_base &) = delete;

  task_promise_base(task_promise_base &&) = delete;

  task_promise_base &operator=(const task_promise_base &) = delete;

  task_promise_base &operator=(task_promise_base &&) = delete;

private:
  std::coroutine_handle<> parent_coro{std::noop_coroutine()};
};

template <typename T> struct task_promise : task_promise_base<T> {
  task_promise() = default;
  ~task_promise() {
    switch (state) {
    case value_state::value: {
      value.~T();
      break;
      ;
    }
    case value_state::exception: {
      exception_ptr.~exception_ptr();
      break;
    }
    default:
      break;
    }
  }

  task<T> get_return_object() {
    return task<T>{std::coroutine_handle<task_promise>::from_promise(*this)};
  }

  void unhandled_exception() {
    exception_ptr = std::current_exception();
    state = value_state::exception;
  }

  template <typename Value>
    requires std::convertible_to<Value &&, T>
  void return_value(Value &&result) {
    std::construct_at(std::addressof(value), std::forward<Value>(result));
    state = value_state::value;
  }

  T &result() & {
    if (state == value_state::exception) {
      std::rethrow_exception(exception_ptr);
    }
    assert(state == value_state::value);
    return value;
  }

  T &&result() && {
    if (state == value_state::exception) {
      std::rethrow_exception(exception_ptr);
    }
    assert(state == value_state::value);
    return std::move(value);
  }

private:
  enum class value_state : uint8_t {
    mono,
    value,
    exception
  } state{value_state::mono};

  union {
    T value;
    std::exception_ptr exception_ptr;
  };
};

template <> struct task_promise<void> : task_promise_base<void> {
  friend struct task_final_awaiter<void>;
  friend class task<void>;
  explicit task_promise() = default;

  ~task_promise() {
    if (is_detached_flag != is_detached) {
      exception_ptr.~exception_ptr();
    }
  };

  task<void> get_return_object();

  void unhandled_exception() {
    if (is_detached_flag == is_detached) {
      std::rethrow_exception(std::current_exception());
    }
    exception_ptr = std::current_exception();
  }

  void result() const {
    if (exception_ptr) {
      std::rethrow_exception(exception_ptr);
    }
  }
      void return_void(){
        
      }

private:
  inline static constexpr uintptr_t is_detached = -1ULL;

  union {
    uintptr_t is_detached_flag{0};
    std::exception_ptr exception_ptr;
  };
};

} // namespace detail


template <typename T = void> class task {
public:
  using promise_type = detail::task_promise<T>;

private:
  struct awaiter_base {
    std::coroutine_handle<promise_type> handle;
    explicit awaiter_base(std::coroutine_handle<promise_type> current)
        : handle(current) {}
    bool await_ready() { return !handle || handle.done(); }
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> awaiting_coro) {
      handle.promise().set_parent(awaiting_coro);
      return handle;
    }
  };

public:
  explicit task(const std::coroutine_handle<promise_type> handle)
      : handle(handle) {}

  task(task &&rhs) noexcept : handle(rhs.handle) { rhs.handle = nullptr; }
  task &operator=(task &&rhs) noexcept {
    if (this != std::addressof(rhs)) {
      if (handle) {
        // handle.destroy();
      }
      handle = rhs.handle;
      rhs.handle = nullptr;
    }
    return *this;
  }

  ~task() {
    if (handle) {
      // handle.destroy();
    }
  }

  task(task &) = delete;
  task &operator=(task &) = delete;

  auto operator co_await() const & {
    struct awaiter : awaiter_base {
      using awaiter_base::awaiter_base;

      decltype(auto) await_resume() { return this->handle.promise().result(); }
    };
    return awaiter{handle};
  }

  auto operator co_await() const && {
    struct awaiter : awaiter_base {
      using awaiter_base::awaiter_base;

      decltype(auto) await_resume() {
        return std::move(this->handle.promise()).result();
      }
    };
    return awaiter{handle};
  }

  std::coroutine_handle<promise_type> get_handle() noexcept { return handle; }

  void detach() noexcept {
    if constexpr (std::is_void_v<T>) {
      handle.promise().is_detached_flag = promise_type::is_detached;
    }
    handle = nullptr;
  }

private:
  std::coroutine_handle<promise_type> handle{};
};

namespace detail {

inline task<void> task_promise<void>::get_return_object() {
  return task<void>{std::coroutine_handle<task_promise>::from_promise(*this)};
}
} // namespace detail

} // namespace coroutine

#endif // TASK_HPP
