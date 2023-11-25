//
// Created by Asukai on 2023/11/22.
//

#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <span>

#include "awaitable.hpp"

namespace coroutine {
class socket {
public:
    socket(int fd):fd_(fd) {}

    detail::co_recv recv(std::span<char> buf) const;
    detail::co_send send(std::span<char> buf, size_t len) const;
private:
    int fd_;
};
}





#endif //SOCKET_HPP
