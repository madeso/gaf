#pragma once

#include <string>
#include <vector>
#include <functional>

#include "pugixml.hpp"

namespace gaf
{
    // return true of ok, false if not
    bool parse_bool(bool* dest, const std::string& value);

    using could_be_fun = std::function<std::string(const std::string&, const std::vector<std::string>&)>;

    std::string could_be_fun_none(const std::string& name, const std::vector<std::string>& values);
    std::string could_be_fun_all(const std::string& name, const std::vector<std::string>& values);

    std::vector<std::string> get_all_atributes(const pugi::xml_node& e);
    std::vector<std::string> get_all_children(const pugi::xml_node& e);
}
