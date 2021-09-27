#include <chrono>
#include <iostream>
#include <utility>

#include "Socket.h"
#include "ThreadPool.h"

using namespace http;
using namespace std::chrono_literals;

void fun(WorkerThread* worker)
{
    int value = 0;
    std::function<void()> meth = []() {
            std::cout << "dupaaaaaaaa" << std::endl;
        };
    while (true)
    {
        std::this_thread::sleep_for(1500ms);
        worker->acceptJob(meth);
    }
}

int main()
{
    WorkerThread thr;
    thr.addObserver([](const std::string& msg){std::cout << "wiadomosc:" << msg << std::endl;});
    std::thread thr2{fun, &thr};
    thr.join();
    return 0;
}
