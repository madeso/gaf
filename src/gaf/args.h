#pragma once

#include <string>
#include <vector>

struct Args
{
    Args(const std::string& a);
    std::string app;
    std::vector<std::string> args;
    std::size_t index = 0;

    std::string peek() const;
    std::string read();

    bool has_more() const;
};
