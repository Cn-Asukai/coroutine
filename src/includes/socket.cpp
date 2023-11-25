//
// Created by Asukai on 2023/11/22.
//

#include "socket.hpp"

namespace coroutine {
detail::co_recv socket::recv(std::span<char> buf) const {
    return detail::co_recv{fd_, buf};
}

detail::co_send socket::send(const std::span<char> buf, const size_t len) const {
    return detail::co_send{fd_, buf, len};
}
}
