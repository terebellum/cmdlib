#ifdef _WIN32
#    include <cmdlib/win32/command_impl.hpp>
#else
#    include <cmdlib/unix/command_impl.hpp>
#endif
// command_tech is the class that gives following functions:
// void start(const vector<std::string>&)
// std::ostream& in()
// void close_in()
// std::istream& out()
// void close_out()
// std::istream& err()
// wait()
