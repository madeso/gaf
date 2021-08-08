#include "gaf/lib_pugixml.h"

#include "fmt/format.h"

namespace gaf
{
    Error::Error(const std::string& d)
        : description(d)
    {
    }

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

    template <>
    std::optional<std::int8_t> parse_number<std::int8_t>(const std::string& value)
    {
        return cast_parse_number<std::int8_t, std::int64_t>(value);
    }

    template <>
    std::optional<std::uint8_t> parse_number<std::uint8_t>(const std::string& value)
    {
        return cast_parse_number<std::uint8_t, std::int64_t>(value);
    }

    std::string could_be_fun_none(const std::string&, const std::vector<std::string>&)
    {
        return "";
    }

    std::string could_be_fun_all(const std::string&, const std::vector<std::string>& values)
    {
        return missing_fun_all(values);
    }

    std::string missing_fun_all(const std::vector<std::string>& values)
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

    std::vector<std::string> get_all_attributes(const pugi::xml_node& e)
    {
        const auto vec = get_all_attributes_set(e);
        return std::vector<std::string>(vec.begin(), vec.end());
    }

    std::vector<std::string> get_all_children(const pugi::xml_node& e)
    {
        const auto vec = get_all_children_set(e);
        return std::vector<std::string>(vec.begin(), vec.end());
    }

    std::set<std::string> get_all_attributes_set(const pugi::xml_node& e)
    {
        std::set<std::string> r;
        for (const auto& a : e.attributes())
        {
            r.emplace(a.name());
        }
        return r;
    }

    std::set<std::string> get_all_children_set(const pugi::xml_node& e)
    {
        std::set<std::string> r;
        for (const auto& a : e.children())
        {
            r.emplace(a.name());
        }
        return r;
    }

    std::string get_path(const pugi::xml_node& ee)
    {
        std::string r = fmt::format("/{}", ee.name());

        pugi::xml_node node = ee.parent();

        // todo(Gustav): add 1-based index...

        while (node.type() != pugi::node_null && node.type() != pugi::node_document)
        {
            r = fmt::format("/{}{}", node.name(), r);
            node = node.parent();
        }

        return r;
    }
}
