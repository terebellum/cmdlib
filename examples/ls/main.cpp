#include <cmdlib/command.hpp>
#include <iostream>

int main(int, char*[])
{
    cmdlib::command cmd("ls");
    std::cout << cmd.output();
    return 0;
}
