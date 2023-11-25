#ifndef READ_HPP
#define READ_HPP

#include "../task.hpp"
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <cstdlib>
#include <fcntl.h>
#include <liburing.h>
#include <span>
#include <string>
#include <sys/types.h>


namespace coroutine {

constexpr std::size_t BUFFER_SIZE = 4096;

auto read_file(const std::string &file_name, std::span<char> buffer) {
  struct awaitable {
    awaitable(std::string file_name, std::span<char> buffer)
    : m_file_name(file_name), m_buffer(buffer)
     {}

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<task::promise_type> h) {
      this->m_h = &h.promise();
      auto uring = h.promise().uring;
      // io_uring_prep_read
      auto sqe = io_uring_get_sqe(uring);
      auto fd = open(m_file_name.c_str(), O_RDONLY);
      assert(fd != -1);
      io_uring_prep_read(sqe, fd, m_buffer.data(), BUFFER_SIZE, 0);
      io_uring_submit(uring);
    }

    ssize_t await_resume() { return m_h->res; }

    std::string m_file_name;
    std::span<char> m_buffer;
    task::promise_type *m_h{};
  };

  return awaitable{file_name, buffer};
}

} // namespace coroutine


#endif // READ_HPP