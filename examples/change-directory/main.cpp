#include <cmdlib/command.hpp>
#include <iostream>

int main(int, char*[])
{
    cmdlib::command cmd("ls");
    cmd.directory = "new_dir";
    std::cout << cmd.output();
    return 0;
}
