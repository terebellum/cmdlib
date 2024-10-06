#ifndef CMDLIB_HANDLE_STREAM_
#define CMDLIB_HANDLE_STREAM_

#include <iostream>
#include <streambuf>

namespace cmdlib {

using write_function = size_t (*)(void*, const void*, size_t);
using read_function = size_t (*)(void*, void*, size_t);

// output descriptor with handle streambuf
class odhbuf: public std::streambuf {
    enum { default_buffer_size = 8192 };

    void* handle_;
    write_function write_function_;
    char* buffer_;
    size_t buffer_size_;

    inline void reset_buffer() { std::streambuf::setp(buffer_, buffer_ + buffer_size_ - 1); }

    static inline bool is_eof(int_type ch) { return traits_type::eq_int_type(ch, traits_type::eof()); }

    void write_bytes(const char* bytes, ssize_t size);

protected:
    virtual int_type overflow(int_type ch = traits_type::eof());
    virtual int sync();
    virtual std::streamsize xsputn(const char*, std::streamsize);

public:
    odhbuf(void* handle, ::cmdlib::write_function write);
    odhbuf(const odhbuf&) = delete;
    odhbuf& operator=(const odhbuf&) = delete;
    ~odhbuf();
};

class odhstream: public std::ostream {
    mutable odhbuf buffer_;

public:
    odhstream(void* handle, write_function write_fun): std::ostream(0), buffer_(handle, write_fun)
    {
        std::ostream::rdbuf(&buffer_);
    }

    odhbuf* rdbuf() const { return &buffer_; }
};

class idhbuf: public std::streambuf {
    enum { default_buffer_size = 8192, putback_size = 4 };

    void* handle_;
    read_function read_func_;
    char* buffer_;
    size_t buffer_size_;

    inline void reset_buffer(size_t nputback, size_t nread)
    {
        std::streambuf::setg(
            buffer_ + (putback_size - nputback), buffer_ + putback_size, buffer_ + putback_size + nread);
    }

    idhbuf(const idhbuf&) = delete;
    idhbuf& operator=(const idhbuf&) = delete;

protected:
    virtual int_type underflow();
    virtual std::streamsize xsgetn(char*, std::streamsize);

public:
    idhbuf(void*, ::cmdlib::read_function);
    ~idhbuf();
};

class idhstream: public std::istream {
    mutable idhbuf buf;

public:
    explicit idhstream(void* handle, read_function read_fun): std::istream(0), buf(handle, read_fun)
    {
        std::istream::rdbuf(&buf);
    }

    idhbuf* rdbuf() const { return &buf; }
};

} // namespace cmdlib

#endif
