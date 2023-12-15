#ifndef PARSER_HPP
#define PARSER_HPP
#include "llhttp.h"
#include <iostream>
#include <mutex>
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

void setting_init(llhttp_settings_t &_setting);

class request;

class parser {
public:
  explicit parser(request &r);

  void parse(std::string_view data) {
    // auto size = data.size();
    auto res = llhttp_execute(&parser_, data.data(), data.size());
    if (res != HPE_OK) {
      std::cout << "parse error!" << llhttp_errno_name(res) << std::endl;
    }
  }

  ~parser() = default;

  llhttp_t parser_{};

  static inline std::once_flag flag_;
  static inline llhttp_settings_t setting_;

  std::string &method_;
  std::string &url_;
  std::string &version_;

  std::vector<std::string> &header_fields_;
  std::vector<std::string> &header_values_;

  std::string &body_;
};

} // namespace coroutine

#endif
