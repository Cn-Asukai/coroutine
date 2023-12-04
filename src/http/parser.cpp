#include "parser.hpp"
#include "request.hpp"
#include <span>
#include <string_view>
#include <vector>

namespace coroutine {
int on_url(llhttp_t *_parser, const char *at, size_t length) {
  auto p = static_cast<parser *>(_parser->data);
  p->url_ = std::string_view{at, length};
  return 0;
}
int on_method(llhttp_t *_parser, const char *at, size_t length) {

  auto p = static_cast<parser *>(_parser->data);
  p->method_ = std::string_view{at, length};
  return 0;
}
int on_version(llhttp_t *_parser, const char *at, size_t length) {

  auto p = static_cast<parser *>(_parser->data);
  p->version_ = std::string_view{at, length};
  return 0;
}

int on_header_field(llhttp_t *_parser, const char *at, size_t length) {

  auto p = static_cast<parser *>(_parser->data);
  p->header_fields_.emplace_back(at, length);
  return 0;
}
int on_header_value(llhttp_t *_parser, const char *at, size_t length) {
  auto p = static_cast<parser *>(_parser->data);
  p->header_values_.emplace_back(at, length);
  return 0;
}

int on_body(llhttp_t *_parser, const char *at, size_t length) {
  auto p = static_cast<parser *>(_parser->data);
  p->body_ = std::string_view{at, length};
  return 0;
}
parser::parser(request &r)
    : method_(r.method_), url_(r.url_), version_(r.version_),
      header_fields_(r.header_fields_), header_values_(r.header_values_)
  ,body_(r.body_)
   {
  llhttp_settings_init(&setting_);
  setting_.on_url = on_url;
  setting_.on_method = on_method;
  setting_.on_version = on_version;
  setting_.on_header_field = on_header_field;
  setting_.on_header_value = on_header_value;
  setting_.on_body = on_body;
  llhttp_init(&parser_, HTTP_REQUEST, &setting_);
  parser_.data = this;
}

} // namespace coroutine
