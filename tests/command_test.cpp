
#include <cmdlib/command.hpp>
#include <istream>
#include <thread>
#include <vector>

#include "command_utils.h"
#include <gtest/gtest.h>

TEST(CommandTest, Cat)
{
    std::vector<std::string> requiredCommands = {"cat"};
    REQUIRE_COMMANDS(requiredCommands);

    cmdlib::command cmd("cat", "test_data/HelloWorld.txt");
    EXPECT_EQ(cmd.output(), "Hello world!\n");
}

TEST(CommandTest, CatDifferentDirectory)
{
    std::vector<std::string> requiredCommands = {"cat"};
    REQUIRE_COMMANDS(requiredCommands);

    cmdlib::command cmd("cat", "HelloWorld.txt");
    cmd.directory = "test_data";
    EXPECT_EQ(cmd.output(), "Hello world!\n");
}

TEST(CommandTest, PropertiesCorrectlySet)
{
    std::vector<std::string> requiredCommands = {"cat"};
    REQUIRE_COMMANDS(requiredCommands);

    std::vector<std::string> expected = {"cat", "test_data/HelloWorld.txt"};
    cmdlib::command cmd(expected);
    EXPECT_EQ(cmd.arguments(), expected);
    EXPECT_EQ(cmd.path(), expected[0]);
    EXPECT_EQ(cmd.directory, "");
}

TEST(CommandTest, Stdin)
{
    std::vector<std::string> requiredCommands = {"sort"};
    REQUIRE_COMMANDS(requiredCommands);

    cmdlib::command cmd("sort");
    std::ostream& in = cmd.in();

    std::thread t([&]() {
        in << "banana\n"
           << "apple\n"
           << "peach\n"
           << '\n';
        cmd.in_close();
    });
    t.detach();

    EXPECT_EQ(cmd.output(), "\napple\nbanana\npeach\n");
}

TEST(CommandTest, Stdout)
{
    std::vector<std::string> requiredCommands = {"echo"};
    REQUIRE_COMMANDS(requiredCommands);

    std::string expected = "hi";

    cmdlib::command cmd("echo", expected);
    std::istream& out = cmd.out();
    cmd.run();

    std::string actual;
    out >> actual;

    EXPECT_EQ(actual, expected);
}

TEST(CommandTest, ReturnCode)
{
    std::vector<std::string> requiredCommands = {"python3"};
    REQUIRE_COMMANDS(requiredCommands);

    cmdlib::command cmd_success("python3", "-c", "import sys; sys.exit(0)");
    cmd_success.run();

    cmdlib::command cmd_error("python3", "-c", "import sys; sys.exit(1)");
    cmd_error.run();

    cmdlib::command cmd_random("python3", "-c", "import sys; sys.exit(42)");
    cmd_random.run();

    EXPECT_EQ(cmd_success.code(), 0);
    EXPECT_EQ(cmd_error.code(), 1);
    EXPECT_EQ(cmd_random.code(), 42);
}

TEST(CommandTest, Stderr)
{
    std::vector<std::string> requiredCommands = {"python3"};
    REQUIRE_COMMANDS(requiredCommands);

    cmdlib::command cmd("python3", "./test_data/echo_err.py");
    std::istream& err = cmd.err();
    cmd.run();

    std::string actual;
    err >> actual;

    EXPECT_EQ(actual, "error");
}
