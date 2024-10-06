#ifndef CMDLIB_UNIX_COMMAND_IMPL_
#define CMDLIB_UNIX_COMMAND_IMPL_

#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include <cmdlib/handle_stream.hpp>

namespace cmdlib {

class dev_null {
    int file = -1;

public:
    dev_null();
    int descriptor() const;
    ~dev_null();
};

const dev_null& get_dev_null();

class command_impl {
    int pid_ = -1;

    int stdin_pipe_reader_ = get_dev_null().descriptor();
    int stdin_pipe_writer_ = -1;
    odhstream* stdin_pipe_ = nullptr;

    int stdout_pipe_reader_ = -1;
    int stdout_pipe_writer_ = get_dev_null().descriptor();
    idhstream* stdout_pipe_ = nullptr;

    int stderr_pipe_reader_ = -1;
    int stderr_pipe_writer_ = get_dev_null().descriptor();
    idhstream* stderr_pipe_ = nullptr;

    static size_t write_to(int descriptor, const void* buf, size_t count);
    static size_t read_from(int descriptor, void* buf, size_t count);

    static size_t write_stdin(void* handle, const void* buf, size_t count);
    static size_t read_stdout(void* handle, void* buf, size_t count);
    static size_t read_stderr(void* handle, void* buf, size_t count);

public:
    command_impl() {};

    command_impl(const command_impl&) = delete;
    command_impl operator=(const command_impl&) = delete;

    ~command_impl();

    void start(const std::vector<std::string>& arguments, const std::string& directory = "");
    int wait();

    std::ostream& in();
    void in_close();

    std::istream& out();
    void out_close();

    std::istream& err();
    void err_close();
};
} // namespace cmdlib

#endif
