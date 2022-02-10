#include <chrono>
#include <iostream>
#include <utility>

#include "Socket.h"
#include "ThreadPool.h"
#include "HttpServer.h"
#include "FileReader.hpp"
#include "HtmlDecoder.hpp"
#include "eventsystem/EventDispatcher.hpp"

using namespace http;
using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

int main()
{
    HttpServer server("0.0.0.0");
    server.registerCallback(HttpMethod::Get, Url("/students"), [](auto)->auto{
        std::cout << "callback processed" << std::endl;
        static int i=0;
        auto iStr = std::to_string(i);
        std::unordered_map<std::string, std::string> data = {{"name", "Rafal"}, {"surname", "Felczynski"}};
        auto htmlContents = FileReader::read("../../pages/index.html");
        htmlContents = HtmlDecoder().replace(htmlContents, data);

        auto msg = "HTTP/1.1 200 OK\r\n"
                    "Cache-Control: no-cache, private\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " + std::to_string(htmlContents.size()) + "\r\n"
                    "\r\n"
                    + htmlContents;
        i++;
        return msg;
     });
    EventDispatcher dis;
    server.run();
    return 0;
}
