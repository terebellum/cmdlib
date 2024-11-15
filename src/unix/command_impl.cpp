#include "cmdlib/handle_stream.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <sched.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>

#include <cmdlib/unix/command_impl.hpp>

namespace cmdlib {

// static

// safe close that doesn't touch global /dev/null
static int safe_close(int& descriptor)
{
    if (get_dev_null().descriptor() == descriptor || descriptor == -1) {
        return -1;
    }
    int ret = close(descriptor);
    descriptor = -1;
    return ret;
}

static void redirect(int descriptor_from, int descriptor_to)
{
    dup2(descriptor_from, descriptor_to);
    safe_close(descriptor_from);
}

static int execute_vector(const std::string& file, const std::vector<std::string>& arguments)
{
    std::vector<const char*> c_arguments;
    c_arguments.reserve(arguments.size());
    for (auto& argument: arguments) {
        c_arguments.push_back(argument.c_str());
    }
    c_arguments.push_back(nullptr);
    return execvp(file.c_str(), const_cast<char**>(c_arguments.data()));
}

// dev_null

dev_null::dev_null()
{
    file = open("/dev/null", O_RDWR);
}

int dev_null::descriptor() const
{
    return file;
}

dev_null::~dev_null()
{
    close(file);
}

const dev_null& get_dev_null()
{
    static dev_null dn;
    return dn;
}

// command_impl

command_impl::~command_impl()
{
    in_close();
    out_close();
    err_close();
}

std::ostream& command_impl::in()
{
    if (stdin_pipe_writer_ == -1) {
        int fd[2];
        pipe(fd);
        stdin_pipe_reader_ = fd[0];
        stdin_pipe_writer_ = fd[1];
        stdin_pipe_ = new odhstream(this, write_stdin);
    }
    return *stdin_pipe_;
}

std::istream& command_impl::out()
{
    if (stdout_pipe_reader_ == -1) {
        int fd[2];
        pipe(fd);
        stdout_pipe_reader_ = fd[0];
        stdout_pipe_writer_ = fd[1];
        stdout_pipe_ = new idhstream(this, read_stdout);
    }
    return *stdout_pipe_;
}

std::istream& command_impl::err()
{
    if (stderr_pipe_reader_ == -1) {
        int fd[2];
        pipe(fd);
        stderr_pipe_reader_ = fd[0];
        stderr_pipe_writer_ = fd[1];
        stderr_pipe_ = new idhstream(this, read_stderr);
    }
    return *stderr_pipe_;
}

void command_impl::in_close()
{
    delete stdin_pipe_;
    stdin_pipe_ = nullptr;
    safe_close(stdin_pipe_reader_);
    safe_close(stdin_pipe_writer_);
}

void command_impl::out_close()
{
    delete stdout_pipe_;
    stdout_pipe_ = nullptr;
    safe_close(stdout_pipe_reader_);
    safe_close(stdout_pipe_writer_);
}

void command_impl::err_close()
{
    delete stderr_pipe_;
    stderr_pipe_ = nullptr;
    safe_close(stderr_pipe_reader_);
    safe_close(stderr_pipe_writer_);
}

void command_impl::start(const std::vector<std::string>& arguments, const std::string& directory)
{
    pid_ = fork();
    if (pid_ == -1) {
        throw std::system_error(errno, std::system_category(), "command_impl::spawn() -> fork()");
    } else if (pid_ == 0) {
        safe_close(stdin_pipe_writer_);
        safe_close(stdout_pipe_reader_);
        safe_close(stderr_pipe_reader_);

        redirect(stdin_pipe_reader_, STDIN_FILENO);
        redirect(stdout_pipe_writer_, STDOUT_FILENO);
        redirect(stderr_pipe_writer_, STDERR_FILENO);

        // change directory
        if (directory != "") {
            if (chdir(directory.c_str()) == -1) {
                throw std::system_error(errno, std::system_category(), "command_impl::spawn() -> chdir()");
            }
        }

        execute_vector(arguments[0], arguments);
        // no info if there are some system error in execution, only that there are error and no output
        // requires extra pipe and i don't sure it is often case
        _exit(-1);
    }
    safe_close(stdin_pipe_reader_);
    safe_close(stdout_pipe_writer_);
    safe_close(stderr_pipe_writer_);
}

int command_impl::wait()
{
    int status;
    if (waitpid(pid_, &status, 0) == -1) {
        throw std::system_error(errno, std::system_category(), "command_impl::wait() -> waitpid()");
    }
    return status;
}

size_t command_impl::write_to(int descriptor, const void* buf, size_t count)
{
    ssize_t ret;
    while ((ret = write(descriptor, buf, count)) == -1 && errno == EINTR) { };
    if (ret < 0) {
        throw std::system_error(errno, std::system_category(), "command_impl::read_from() -> write");
    }
    return static_cast<size_t>(ret);
}

size_t command_impl::read_from(int descriptor, void* buf, size_t count)
{
    ssize_t ret;
    while ((ret = read(descriptor, buf, count)) == -1 && errno == EINTR) { };
    if (ret < 0) {
        throw std::system_error(errno, std::system_category(), "command_impl::read_from() -> read");
    }
    return static_cast<size_t>(ret);
}

size_t command_impl::write_stdin(void* handle, const void* buf, size_t count)
{
    return write_to(static_cast<command_impl*>(handle)->stdin_pipe_writer_, buf, count);
}

size_t command_impl::read_stdout(void* handle, void* buf, size_t count)
{
    return read_from(static_cast<command_impl*>(handle)->stdout_pipe_reader_, buf, count);
}

size_t command_impl::read_stderr(void* handle, void* buf, size_t count)
{
    return read_from(static_cast<command_impl*>(handle)->stderr_pipe_reader_, buf, count);
}

} // namespace cmdlib
