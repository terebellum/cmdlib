#include <cmdlib/command.hpp>
#include <ostream>
#include <sstream>
#include <thread>

std::string read_stream(std::istream& is)
{
    std::stringstream ss;
    ss << is.rdbuf();
    return ss.str();
}

int main(int, char*[])
{
    cmdlib::command cmd("python", "inouterr.py");
    std::ostream& in = cmd.in();
    std::istream& out = cmd.out();
    std::istream& err = cmd.err();
    cmd.start();

    std::thread t([&]() {
        in << "stdin from c++" << '\n';
        // neccesary if program reads till eof
        cmd.in_close();
    });

    std::cout << '\n' << "stdout:" << '\n' << read_stream(out) << "\n";
    std::cout << '\n' << "stderr:" << '\n' << read_stream(err) << "\n";

    cmd.wait();

    t.detach();
    return 0;
}
