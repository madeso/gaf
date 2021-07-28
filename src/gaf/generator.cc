#include "gaf/generator.h"

#include <cassert>
#include <iostream>
#include <filesystem>

#include "fmt/format.h"


void Lines::add(const std::string& str)
{
    const auto has_newline = str.find('\n') != std::string::npos;
    const auto starts_with_space = str.find_first_not_of(" ") != 0;
    const auto is_valid = str.empty() || (!has_newline && !starts_with_space);
    if (!is_valid)
    {
        int i = 0;
        i += 1;
        std::cerr << "failed <" << str << ">\n";
    }

    assert(is_valid);
    lines.emplace_back(str);
}

void Lines::addfv(fmt::string_view format, fmt::format_args args)
{
    add(fmt::vformat(format, args));
}

std::string get_file_path(const std::string& folder, const std::string& name)
{
    const auto r = std::filesystem::path{folder} / std::filesystem::path{name};
    const auto a = std::filesystem::absolute(r);
    return a.string();
}

void write_lines(const Lines& lines, Writer* writer, const std::string& path)
{
    if (auto out = writer->open(path); out != nullptr)
    {
        for (const auto& s : lines.lines)
        {
            out->write(s);
        }
    }
}

Lines complete_source(const Lines& source, const std::string& name, const std::string& prefix, const std::string& package_name, const std::vector<std::string>& includes)
{
    Lines ret;

    ret.addf("#include \"{}.h\"", prefix + name);
    ret.add("");
    for (const auto& inc : includes)
    {
        ret.addf("#include {}", inc);
    }
    ret.add("");

    if (package_name.empty() == false)
    {
        ret.addf("namespace {} {{", package_name);
        ret.add("");
    }

    for (const auto& s : source.lines)
    {
        ret.add(s);
    }

    if (package_name.empty() == false)
    {
        ret.add("}");
        ret.add("");
    }

    return ret;
}

void write_cpp(Out* sources, Writer* writer, const std::string& out_dir, const std::string& name, const std::string& prefix, const std::string& package_name, const std::vector<std::string>& includes)
{
    write_lines(sources->header, writer, get_file_path(out_dir, prefix + name + ".h"));
    write_lines(complete_source(sources->source, name, prefix, package_name, includes), writer, get_file_path(out_dir, prefix + name + ".cc"));
}
