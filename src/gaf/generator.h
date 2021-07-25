#pragma once

#include <vector>
#include <string>

#include "gaf/types.h"

struct Lines
{
    std::vector<std::string> lines;

    void add(const std::string& str);
};

struct Out
{
    Lines header;
    Lines source;
};

void write_cpp(Out* sources, Writer* writer, const std::string& out_dir, const std::string& name, const std::string& prefix, const std::string& package_name, const std::vector<std::string>& includes);
