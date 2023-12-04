#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "parser.hpp"
#include <span>

namespace coroutine {

class request {
public:
  explicit request(std::span<char> buf) {
    parser_.parse({buf.data(), buf.size()});
  }

  parser parser_{*this};
  std::string method_;
  std::string url_;
  std::string version_;

  std::vector<std::string> header_fields_;
  std::vector<std::string> header_values_;

  std::string body_;
};
} // namespace coroutine

#endif
