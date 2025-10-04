#include <iostream>
#include "register.h"
#include <unistd.h>

const std::string first_server_ip = "127.0.0.1";  // 先用localhost测试
const short first_server_port = 8080;

int main()
{
    Register regis(first_server_ip, first_server_port);
    if (regis.start() == -1) {
        std::cerr << "Failed to start register, exiting..." << std::endl;
        return -1;
    }

    while (true)
    {
        regis.heart_info_to_server();
        sleep(5);  // 防止CPU过度占用
    }

    return 0;
}