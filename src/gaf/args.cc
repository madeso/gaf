#include "gaf/args.h"

#include <iostream>

Args::Args(const std::string& a)
    : app(a)
{
}

std::string Args::peek() const
{
    if (index >= args.size())
    {
        return "";
    }
    return args[index];
}

std::string Args::read()
{
    const auto r = peek();
    index += 1;
    return r;
}

bool Args::has_more() const
{
    return index < args.size();
}

int no_arguments(Args& args)
{
    while (args.has_more())
    {
        const auto r = args.read();
        std::cerr << "invalid argument " << r << "\n";
        return -42;
    }

    return 0;
}

bool is_option(const std::string& str)
{
    if (str.empty() == false)
    {
        if (str[0] == '-')
        {
            return true;
        }
    }

    return false;
}
