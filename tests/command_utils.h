#ifndef COMMAND_UTILS_H
#define COMMAND_UTILS_H

#include <cstdlib>
#include <string>
#include <vector>

inline bool commandExists(const std::string& command)
{
#ifdef _WIN32
    std::string cmd = "where " + command + " >NUL 2>NUL";
#else
    std::string cmd = "command -v " + command + " >/dev/null 2>&1";
#endif
    int result = std::system(cmd.c_str());
    return result == 0;
}

inline std::vector<std::string> getMissingCommands(const std::vector<std::string>& commands)
{
    std::vector<std::string> missingCommands;
    for (const auto& cmd: commands) {
        if (!commandExists(cmd)) {
            missingCommands.push_back(cmd);
        }
    }
    return missingCommands;
}

#define REQUIRE_COMMANDS(commands)                              \
    do {                                                        \
        auto missing = getMissingCommands(commands);            \
        if (!missing.empty()) {                                 \
            std::string message = "Missing required commands:"; \
            for (const auto& cmd: missing) {                    \
                message += " " + cmd;                           \
            }                                                   \
            GTEST_SKIP() << message;                            \
        }                                                       \
    } while (0)

#endif // COMMAND_UTILS_H