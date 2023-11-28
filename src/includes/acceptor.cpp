//
// Created by Asukai on 2023/11/22.
//

#include "acceptor.hpp"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <sys/socket.h>

namespace coroutine {
acceptor::acceptor(int port) {
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  int opt{1};
  setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  int res = bind(listen_fd_, (sockaddr *)&addr, sizeof(addr));
  assert(res == 0);
  res = listen(listen_fd_, LISTEN_NUM);
  assert(res == 0);
}

detail::co_accept acceptor::accept() const {
  return detail::co_accept{listen_fd_, nullptr, nullptr};
}
} // namespace coroutine
