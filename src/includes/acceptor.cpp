//
// Created by Asukai on 2023/11/22.
//

#include "acceptor.hpp"

#include <assert.h>
#include <arpa/inet.h>
#include <sys/socket.h>


namespace coroutine {
acceptor::acceptor(int port) {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);


    int res = bind(listen_fd_, (sockaddr*)&addr, sizeof(addr));
    assert(res == 0);
    res = listen(listen_fd_, LISTEN_NUM);
    assert(res == 0);
}

detail::co_accept acceptor::accept() const {
    return detail::co_accept{listen_fd_, nullptr, nullptr};
}
}
