#include "awaitable.hpp"
#include "context.hpp"

using namespace std;
using namespace coroutine;

task<> func() {
  while (true) {
    co_await detail::co_timeout(1s);
    cout << "Hello World" << endl;
  }
}

task<> func2() {
  while (true) {
    co_await detail::co_timeout(2s);
    cout << "nihao shijie" << endl;
  }
}

int main() {
  context c;
  c.co_spawn(func());
  c.co_spawn(func2());
  c.start();
  c.join();
}
