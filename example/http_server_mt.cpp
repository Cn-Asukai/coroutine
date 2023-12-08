#include "context.hpp"
#include "http/request.hpp"
#include "net/acceptor.hpp"
#include "net/socket.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace coroutine;
using namespace std;
class web_server {
public:
  explicit web_server(int port) : actor_(port) {}

  void start() {
    ctx_.co_spawn(serve());
    for (auto &w : works) {
      w.start();
    }
    ctx_.start();
  }

  ~web_server() {
    for (auto &w : works) {
      w.join();
    }
    ctx_.join();
  }

private:
  task<> serve() {
    int cur = 0;
    while (true) {
      int fd = co_await actor_.accept();

      works[cur++].co_spawn(session(fd));
      if (cur == 4) {
        cur = 0;
      }
    }
    exit(1);
  }
  task<> session(int fd) {
    coroutine::socket s(fd);
    vector<char> buf(1024, 0);
    try {
      auto n = co_await s.recv(buf);
      request r;
      r.parse_request(span(buf.begin(), n + 1));
      string res{};
      // string file_name;
      // try {
      //   file_name = string{r.url_.begin() + 1, r.url_.end()};
      // } catch (const std::exception &e) {
      //   std::cerr << e.what() << '\n';
      // }
      string file_name{r.url_.begin() + 1, r.url_.end()};

      filesystem::path p("./" + file_name);
      auto size = filesystem::file_size(p);
      // cout << "file_name:" << file_name << endl;
      res += "HTTP/1.1 200 OK\r\n";
      res += "Content-Length: " + to_string(size) + "\r\n";
      res += "\r\n";
      read_file(file_name, res);
      res += "\r\n";
      n = co_await s.send(res, res.size());
      co_await s.close();
    } catch (const std::exception &e) {
      cout << e.what() << endl;
      cout << buf.data() << endl;
    }
  }

  static void read_file(string_view file_name, string &str) {
    fstream f;
    f.open(string{file_name});
    assert(f.is_open());
    string temp;
    while (getline(f, temp)) {
      str += temp;
      str += "\n";
    }
    f.close();
  }

  context ctx_, works[4];
  acceptor actor_;
};

int main() {
  try {
    web_server s(8081);
    s.start();
  } catch (const exception &e) {
    std::cout << e.what() << std::endl;
  }
}
