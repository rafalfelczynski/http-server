#include <chrono>
#include <iostream>
#include <utility>

#include "Socket.h"
#include "ThreadPool.h"
#include "HttpServer.h"

using namespace http;
using namespace std;
using namespace std::chrono;
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

int findLastCharFromRight(const std::string& s, char ch)
{
    int index = s.size() - 1;
    while (index >= 0 && s[index] == ch)
    {
        --index;
    }
    return index+1;
}

int main()
{
    HttpServer server{"localhost"};
    server.registerCallback(HttpMethod::Get, Url("/students"), [](auto)->auto{ std::cout << "wiadomosc" << std::endl; return HttpResponse{}; });
    server.run();
    return 0;
}
