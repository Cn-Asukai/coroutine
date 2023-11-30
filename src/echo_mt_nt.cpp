#include "includes/acceptor.hpp"
#include "includes/context.hpp"
#include "includes/socket.hpp"
#include "includes/task.hpp"
#include <iostream>
#include <vector>

using namespace std;
using namespace coroutine;

context c, works[5];

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
  int cur = 0;
  while (1) {
    int fd = co_await t.accept();
    cout << "fd:" << fd << endl;
    works[cur++].co_spawn(echo(fd));
    if(cur == 5) {
      cur = 0;
    }
  }
}

int main() {
  c.co_spawn(func());
  for(auto& word : works){
    word.start();
  }
  c.start();
  c.join();
}
