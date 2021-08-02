#include "gaf/lib_pugixml.h"

namespace gaf
{
    bool parse_bool(bool* dest, const std::string& value)
    {
        if (value == "true")
        {
            *dest = true;
            return true;
        }
        if (value == "false")
        {
            *dest = false;
            return true;
        }
        return false;
    }

    std::string could_be_fun_none(const std::string&, const std::vector<std::string>&) { return ""; }

    std::string could_be_fun_all(const std::string&, const std::vector<std::string>& values)
    {
        std::string r;
        bool first = true;
        for (const auto& v : values)
        {
            if (first)
            {
                r += v;
                first = false;
            }
            else
            {
                r += ", " + v;
            }
        }
        return r;
    }

    std::vector<std::string> get_all_atributes(const pugi::xml_node& e)
    {
        std::vector<std::string> r;
        for (const auto& a : e.attributes()) { r.emplace_back(a.name()); }
        return r;
    }

    std::vector<std::string> get_all_children(const pugi::xml_node& e)
    {
        std::vector<std::string> r;
        for (const auto& a : e.children()) { r.emplace_back(a.name()); }
        return r;
    }
}
