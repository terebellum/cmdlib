#ifndef CMDLIB_COMMAND_
#define CMDLIB_COMMAND_

#include <initializer_list>
#include <string>

#include <cmdlib/command_impl.hpp>
#include <cmdlib/export.hpp>
#include <concepts>
#include <vector>

namespace cmdlib {

CMDLIB_EXPORT class command {
    command_impl impl_;

    std::string path_;
    std::vector<std::string> arguments_;
    int code_ = 0;

public:
    std::string directory;

    command(const std::vector<std::string>& arguments);
    command(const std::convertible_to<std::string> auto&... args): command(std::vector<std::string>{args...}) { }
    command(std::initializer_list<std::string> args): command(std::vector<std::string>(args)) { }

    std::string path() const;
    std::vector<std::string> arguments() const;
    int code() const;

    void start();
    void wait();
    void run();
    std::string output();

    std::ostream& in();
    void in_close();

    std::istream& out();
    void out_close();

    std::istream& err();
    void err_close();
};

} // namespace cmdlib

#endif
