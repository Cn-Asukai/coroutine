//
// Created by Asukai on 2023/11/22.
//

#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP
#include "awaitable.hpp"

namespace coroutine {

class acceptor {
    static constexpr int LISTEN_NUM = 5;
public:
    explicit acceptor(int port);

    detail::co_accept accept() const;
private:
    int listen_fd_{};
};
}

#endif //ACCEPTOR_HPP
