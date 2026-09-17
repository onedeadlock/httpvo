#include <string>
#include <iostream>
#include <print>
#include "../../implementation.hpp"

using namespace httpvo;

void error(std::string&& msg,  std::size_t exp, std::size_t got)
{
    std::cerr << "(" << msg << ") expected " << exp << " got " << got << std::endl;
        exit(-1);
}

void test(void)
{
    std::string request  = "GET ./index HTTP/1.1 ";
    std::string response = "HTTP/1.1 200 OK ";
    std::size_t i = 0;
    Implementation::ReqLine x{0};
   
    x.request();
    for (auto c : request)
    {
        if (common::is_whitespace(c))
        {
            x.post() += i;
        }
        i++;
    }

    if (x.start_of_version() != 12)
        error("start index of version is incorrect", 12, x.start_of_version());

    if (x.version_size() != 8)
        error("size of version is incorrect", 8, x.version_size());
    
    if (x.start_of_method_msg() != 0)
        error("start index of method is incorrect", 0, x.start_of_method_msg());

    if (x.method_or_msg_size() != 3)
        error("size of version is incorrect", 3, x.method_or_msg_size());

    if (x.start_of_status_uri() != 4)
        error("start index of version is incorrect", 4, x.start_of_status_uri());

    if (x.status_uri_size() != 7)
        error("size of version is incorrect", 7, x.status_uri_size());

    x.response();
    i = 0;
    for (auto c : response)
    {
        if (common::is_whitespace(c))
        {
            x.post() += i;
        }
        i++;
    }

    if (x.start_of_version() != 0)
        error("start index of version is incorrect", 0, x.start_of_version());

    if (x.version_size() != 8)
        error("size of version is incorrect", 8, x.version_size());
    
    if (x.start_of_method_msg() != 13)
        error("start index of method is incorrect", 13, x.start_of_method_msg());

    if (x.method_or_msg_size() != 2)
        error("size of version is incorrect", 2, x.method_or_msg_size());

    if (x.start_of_status_uri() != 9)
        error("start index of version is incorrect", 9, x.start_of_status_uri());

    if (x.status_uri_size() != 3)
        error("size of version is incorrect", 3, x.status_uri_size());
}

int main(void)
{
    test();
    return 0;
}