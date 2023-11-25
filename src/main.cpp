#include <iostream>
#include <string>
#include <vector>

#include "includes/acceptor.hpp"
#include "includes/context.hpp"
#include "includes/socket.hpp"
using namespace std;
using namespace coroutine;

task func() {
    acceptor t{8081};
    int fd = co_await t.accept();
    cout << "fd:" << fd << endl;
    coroutine::socket conn(fd);
    vector<char> buf(1024, 0);
    int n = co_await conn.recv(buf);
    cout << "n:" << n << endl;
    cout << buf.data() << endl;
    n = co_await conn.send(buf, n);
    cout << "n:" << n << endl;
}

int main() {
    context c;
    func();
    c.start();
}
