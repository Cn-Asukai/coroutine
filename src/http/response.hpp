#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <unordered_map>
class response{
public:
  std::string version_{"HTTP/1.1"};
  int code_;
  std::string state_;
  std::unordered_map<std::string, std::string> headers_;
  std::string body_;
};

#endif
