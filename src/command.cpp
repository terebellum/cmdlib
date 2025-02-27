#include <cmdlib/command.hpp>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace cmdlib {

command::command(const std::vector<std::string>& args)
{
    if (args.size() == 0) {
        throw std::runtime_error("cmdlib::command: args vector should not be empty");
    }

    path_ = args[0];
    arguments_ = args;
}

std::string command::path() const
{
    return path_;
}

std::vector<std::string> command::arguments() const
{
    return arguments_;
}

int command::code() const
{
    return code_;
}

void command::start()
{
    impl_.start(arguments_, directory);
}

void command::wait()
{
    code_ = impl_.wait();
}

void command::run()
{
    start();
    wait();
}

std::string command::output()
{
    std::istream& os = out();
    run();
    std::stringstream ss;
    ss << os.rdbuf();
    return ss.str();
}

std::ostream& command::in()
{
    return impl_.in();
}

void command::in_close()
{
    impl_.in_close();
}

std::istream& command::out()
{
    return impl_.out();
}

void command::out_close()
{
    return impl_.out_close();
}

std::istream& command::err()
{
    return impl_.err();
}

void command::err_close()
{
    return impl_.err_close();
}
} // namespace cmdlib
