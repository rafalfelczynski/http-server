#include <chrono>
#include <iostream>
#include <utility>

#include "Socket.h"
#include "ThreadPool.h"
#include "HttpServer.h"
#include "FileReader.hpp"
#include "HtmlDecoder.hpp"
#include "eventsystem/EventDispatcher.hpp"

#include "HtmlHeaders/TypeMap.h"
#include <ranges>
#include <string_view>

using namespace http;
using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

int main()
{
    // HttpServer server("0.0.0.0");
    // server.registerCallback(HttpMethod::Get, Url("/students"), [](const auto&)->auto{
    //     std::cout << "callback processed" << std::endl;
    //     static int i=0;
    //     auto iStr = std::to_string(i);
    //     std::unordered_map<std::string, std::string> data = {{"name", "Rafal"}, {"surname", "Felczynski"}};
    //     auto htmlContents = FileReader::read("../../pages/index.html");
    //     htmlContents = HtmlDecoder().replace(htmlContents, data) + " " + iStr;

    //     auto msg = "HTTP/1.1 200 OK\r\n"
    //                 "Cache-Control: no-cache, private\r\n"
    //                 "Content-Type: text/html\r\n"
    //                 "Content-Length: " + std::to_string(htmlContents.size()) + "\r\n"
    //                 "\r\n"
    //                 + htmlContents;
    //     i++;
    //     return msg;
    //  });
    // server.run();

    //std::this_thread::sleep_for(std::chrono::milliseconds(5000));



    std::string_view s{"linia1"};

    auto delim = std::string_view("..");

    auto spl = s | std::views::split(delim) | std::views::transform([](const auto &view){
        std::cout << "view" << std::endl;
        return std::string_view(view.begin(), view.end());
    });

    for(const auto& ss : spl)
    {
        std::cout << "ss: " << ss << " size: " << ss.size() << std::endl;
    }
    std::cout << "empty? " << spl.empty() << std::endl;


    return 0;
}
