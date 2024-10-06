#include <cmdlib/command.hpp>
#include <iostream>

int main(int, char*[])
{

    // for unix systems you could use shell scripts
    // cmdlib::comand cmd("/bin/bash", "/your/script/location");

    // python should be in path
    cmdlib::command cmd("python", "hi.py");
    std::cout << cmd.output();
    return 0;
}
