#include <cmdlib/handle_stream.hpp>
#include <cstddef>
#include <cstring>

namespace cmdlib {

odhbuf::odhbuf(void* handle, write_function write_func):
    handle_(handle),
    write_function_(write_func),
    buffer_(new char[default_buffer_size]),
    buffer_size_(default_buffer_size)
{
    reset_buffer();
}

odhbuf::~odhbuf()
{
    if (handle_) {
        // destructor is noexpect
        try {
            sync();
        } catch (...) { }
    }
    delete[] buffer_;
}

void odhbuf::write_bytes(const char* bytes, ssize_t size)
{
    while (size > 0) {
        // Invariant: write_function throws on error
        const size_t bytes_written = write_function_(handle_, bytes, static_cast<size_t>(size));
        size -= static_cast<ssize_t>(bytes_written);
        bytes += bytes_written;
    }
}

odhbuf::int_type odhbuf::overflow(odhbuf::int_type c)
{
    const char* p = pbase();
    std::streamsize bytes_to_write = pptr() - p;

    if (!is_eof(c)) {
        *pptr() = static_cast<char_type>(c);
        ++bytes_to_write;
    }

    write_bytes(p, bytes_to_write);
    reset_buffer();

    return traits_type::to_int_type(0);
}

int odhbuf::sync()
{
    return !is_eof(overflow(traits_type::eof())) ? 0 : -1;
}

std::streamsize odhbuf::xsputn(const char* s, std::streamsize n)
{
    // Write directly only if n >= MIN(4096, available buffer capacity)
    if (n < std::min<std::streamsize>(4096, epptr() - pptr())) {
        return std::streambuf::xsputn(s, n);
    }

    // Before we can do a direct write of this string, we need to flush
    // out the current contents of the buffer.
    if (pbase() != pptr()) {
        overflow(traits_type::eof());
    }

    write_bytes(s, n);

    return n; // Return the total bytes written
}

// idhstream

idhbuf::idhbuf(void* handle, ::cmdlib::read_function read_func):
    handle_(handle),
    read_func_(read_func),
    buffer_(new char[default_buffer_size + putback_size]),
    buffer_size_(default_buffer_size)
{
    reset_buffer(0, 0);
}

idhbuf::~idhbuf()
{
    delete[] buffer_;
}

idhbuf::int_type idhbuf::underflow()
{
    if (gptr() >= egptr()) { // A true underflow (no bytes in buffer left to read)

        // Move the putback_size most-recently-read characters into the putback area
        size_t nputback = std::min<size_t>(static_cast<size_t>(gptr() - eback()), putback_size);
        std::memmove(buffer_ + (putback_size - nputback), gptr() - nputback, nputback);

        // Now read new characters from the file descriptor
        const size_t nread = read_func_(handle_, buffer_ + putback_size, buffer_size_);
        if (nread == 0) {
            // EOF
            return traits_type::eof();
        }

        // Reset the buffer
        reset_buffer(nputback, nread);
    }

    // Return the next character
    return traits_type::to_int_type(*gptr());
}

std::streamsize idhbuf::xsgetn(char* s, std::streamsize n)
{
    // Read directly only if n >= bytes_available + 4096

    std::streamsize bytes_available = egptr() - gptr();

    if (n < bytes_available + 4096) {
        // Not worth it to do a direct read
        return std::streambuf::xsgetn(s, n);
    }

    std::streamsize total_bytes_read = 0;

    // First, copy out the bytes currently in the buffer
    std::memcpy(s, gptr(), static_cast<size_t>(bytes_available));

    s += bytes_available;
    n -= bytes_available;
    total_bytes_read += bytes_available;

    // Now do the direct read
    while (n > 0) {
        const size_t bytes_read = read_func_(handle_, s, static_cast<size_t>(n));
        if (bytes_read == 0) {
            // EOF
            break;
        }

        s += bytes_read;
        n -= static_cast<ssize_t>(bytes_read);
        total_bytes_read += static_cast<ssize_t>(bytes_read);
    }

    // Fill up the putback area with the most recently read characters
    size_t nputback = std::min<size_t>(static_cast<size_t>(total_bytes_read), putback_size);
    std::memcpy(buffer_ + (putback_size - nputback), s - nputback, nputback);

    // Reset the buffer with no bytes available for reading, but with some putback characters
    reset_buffer(nputback, 0);

    // Return the total number of bytes read
    return total_bytes_read;
}

} // namespace cmdlib
