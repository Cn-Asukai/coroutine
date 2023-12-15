#include "awaitable.hpp"
#include "context.hpp"
#include "http/request.hpp"
#include "net/acceptor.hpp"
#include "net/socket.hpp"
#include "task.hpp"
#include <cassert>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
using namespace coroutine;
using namespace std;
class web_server {
public:
  explicit web_server(int port) : actor_(port) {}

  void start() {
    ctx_.co_spawn(serve());
    ctx_.co_spawn(request_per_second());
    for (auto i = 0uz; i < size(works); i++) {
      works[i].start();
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
      fds++;

      nums[cur]++;
      works[cur++].co_spawn(session(fd));
      if (cur == size(works)) {
        cur = 0;
      }
    }
    exit(1);
  }
  task<> session(int fd) {
    int cur_size = 0, last_size = 0;
    coroutine::socket s(fd);
    vector<char> buf(1024, 0);
    try {
      auto n = co_await s.recv(buf);
      request r;
      r.parse_request(span(buf.begin(), n));
      string res{};

      string file_name{r.url_.begin() + 1, r.url_.end()};

      filesystem::path p("./" + file_name);
      auto size = filesystem::file_size(p);

      res += "HTTP/1.1 200 OK\r\n";
      res += "Content-Length: " + to_string(size) + "\r\n";
      res += "\r\n";
      co_await read_file(file_name, size, res);
      res += "\r\n";
      cur_size = res.size();
      n = co_await s.send(res, res.size());
      last_size = res.size();
      co_await s.close();
    } catch (const std::exception &e) {
    }
  }

  static task<> read_file(string_view file_name, size_t size, string &str) {
    int fd = open(file_name.data(), O_RDONLY);
    size_t left_size = size;
    vector<char> v(1024, 0);
    if (fd < 0) {
      // cout << "file open error!" << endl;
    }
    while (left_size > 0) {
      memset(v.data(), 0, 1024);
      size_t need_read = left_size > 1024 ? 1024 : left_size;
      int n = co_await detail::co_read(fd, v, need_read);
      left_size -= n;
      str += v.data();
    }
    // cout << "size=" << size << endl;
    close(fd);
    // std::cout << cur_size << endl;
    co_return;
  }

  task<> request_per_second() {
    while (true) {
      co_await detail::co_timeout(1s);
      for (auto i = 0uz; i < size(nums); i++) {
        cout << "thread[" << i << "]: " << nums[i] << endl;
        nums[i] = 0;
      }
    }
  }

  context ctx_, works[15];
  unsigned int nums[15]{};
  acceptor actor_;
  unsigned long long fds = 0;
  jthread t;
};

int main() {

  web_server s(8081);
  s.start();
}
