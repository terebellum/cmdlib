#ifndef CMDLIB_COMMAND_
#define CMDLIB_COMMAND_

#include "cmdlib/unix/command-impl.hpp"
#include <initializer_list>
#include <string>

#include <cmdlib/command-impl.hpp>
#include <cmdlib/export.hpp>
#include <concepts>
#include <vector>

namespace cmdlib {

CMDLIB_EXPORT class command {
    command_impl impl_;

    std::string path_;
    std::vector<std::string> arguments_;
    std::string directory_;

public:
    command(const std::vector<std::string>& arguments);
    command(const std::convertible_to<std::string> auto&... args): command(std::vector<std::string>{args...}) { }
    command(std::initializer_list<std::string> arguments);

    std::string get_path() const;
    std::vector<std::string> get_arguments() const;

    std::string get_directory() const;
    void set_directory(const std::string& directory);

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
