#pragma once

#include <vector>
#include <string>

#include "gaf/types.h"

#include "fmt/format.h"

struct Lines
{
    std::vector<std::string> lines;

    void add(const std::string& str);

    void addfv(fmt::string_view format, fmt::format_args args);

    template <typename S, typename... Args>
    void addf(const S& format, Args&&... args)
    {
        return addfv(format, fmt::make_args_checked<Args...>(format, args...));
    }
};

struct Out
{
    Lines header;
    Lines source;

    void add(const std::string& str);

    void addfv(fmt::string_view format, fmt::format_args args);

    template <typename S, typename... Args>
    void addf(const S& format, Args&&... args)
    {
        return addfv(format, fmt::make_args_checked<Args...>(format, args...));
    }
};

void write_cpp(Out* sources, Writer* writer, const std::string& out_dir, const std::string& name,
               const std::string& prefix);
