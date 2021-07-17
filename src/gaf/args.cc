#include "gaf/args.h"

Args::Args(const std::string& a)
    : app(a)
{
}

std::string Args::peek() const
{
    if(index >= args.size())
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
