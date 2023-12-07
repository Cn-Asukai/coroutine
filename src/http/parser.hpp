#ifndef PARSER_HPP
#define PARSER_HPP
#include "llhttp.h"
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>
namespace coroutine {

int on_url(llhttp_t *, const char *at, size_t length);
int on_method(llhttp_t *, const char *at, size_t length);
int on_version(llhttp_t *, const char *at, size_t length);

int on_header_field(llhttp_t *, const char *at, size_t length);
int on_header_value(llhttp_t *, const char *at, size_t length);
int on_body(llhttp_t *_parser, const char *at, size_t length);

class request;

class parser {
public:
  explicit parser(request &r);

  void parse(std::string_view data) {
    auto res = llhttp_execute(&parser_, data.data(), data.size());
    if (res != HPE_OK) {
      std::cout << "parse error!" << __LINE__ << std::endl;
    }
  }

  ~parser() {}

  llhttp_t parser_{};
  inline static llhttp_settings_t setting_;

  std::string &method_;
  std::string &url_;
  std::string &version_;

  std::vector<std::string> &header_fields_;
  std::vector<std::string> &header_values_;

  std::string &body_;
};

} // namespace coroutine

#endif
