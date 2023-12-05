#ifndef HTTP_SESSION_HPP
#define HTTP_SESSION_HPP

#include "net/socket.hpp"
#include "request.hpp"
#include "response.hpp"


namespace coroutine {

class http_session {
public:
    explicit http_session(int fd) : socket_(fd){}
private:
    request req_;
    response res_;
    socket socket_;
};

} // namespace coroutine

#endif
