#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "context.hpp"
#include "http/request.hpp"
#include "net/acceptor.hpp"
#include "net/socket.hpp"
using namespace std;
using namespace coroutine;

context c;

task<> echo(int fd) {
  coroutine::socket conn(fd);
  vector<char> buf(1024, 0);

  int n = co_await conn.recv(buf);
  if (n <= 0) {
    std::cout << n << endl;
    co_await conn.close();
    std::cout << "client quit!" << endl;
    co_return;
  }
  request r{{buf.data(), (size_t)n}};
  cout << "url: " << r.url_ << endl;
  cout << "version: " << r.version_ << endl;
  cout << "method: " << r.method_ << endl;
  for(int i = 0; i < r.header_fields_.size(); i++) {
    cout << r.header_fields_[i] << " : " << r.header_values_[i] << endl;
  }
  cout << r.body_ << endl;
}

task<> func() {
  acceptor t{8081};
  while (1) {
    int fd = co_await t.accept();
    co_spawn(echo(fd));
  }
}

int main() {
  c.co_spawn(func());
  c.start();
  c.join();
}
