#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "net/acceptor.hpp"
#include "context.hpp"
#include "net/socket.hpp"
using namespace std;
using namespace coroutine;

context c;

task<> echo(int fd) {

  coroutine::socket conn(fd);
  vector<char> buf(1024, 0);
  while (1) {

    int n = co_await conn.recv(buf);
    if (n <= 0) {
      std::cout << n << endl;
      co_await conn.close();
      std::cout << "client quit!" << endl;
      co_return;
    }
    cout << "n:" << n << endl;
    cout << buf.data() << endl;
    n = co_await conn.send(buf, n);
    cout << "n:" << n << endl;
    fill(buf.begin(), buf.end(), 0);
  }
}

task<> func() {
  acceptor t{8081};
  while (1) {
    int fd = co_await t.accept();
    cout << "fd:" << fd << endl;
    co_spawn(echo(fd));
  }
}

int main() {
  c.co_spawn(func());
  c.start();
  c.join();
}
