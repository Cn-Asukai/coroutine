#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "context.hpp"
#include <vector>

namespace coroutine {
class http_server {
public:
private:
  std::vector<context> contexts_;
};
} // namespace coroutine

#endif
