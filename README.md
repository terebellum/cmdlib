# cmdlib:  C++ commands execution library

`cmdlib` is a collection of tools that provides simple and ergonomic way of executing shell commands.

## Example

```c++
#include <iostream>
#include <cmdlib/command.hpp>

int main(int, char*[])
{
    // Let's execute ls command
    cmdlib::command cmd("ls", "dir");
    std::cout << cmd.output();

    // You could choose directory of execution if you want
    cmdlib::command cmd("ls");
    cmd.directory = "dir";
    std::cout << cmd.output();

    // stdin/stdout/stderr streams also supported
    cmdlib::command cmd("sort");
    std::ostream& in = cmd.in();

    std::thread t([&](){
        in << "banana\n" << "apple\n" << "peach\n"  << '\n';
        cmd.in_close();
    });

    t.detach();

    std::cout << cmd.output() << '\n';
    return 0;
}
```
