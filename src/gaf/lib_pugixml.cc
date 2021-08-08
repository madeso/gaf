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

    std::string get_path_of_attribute(const pugi::xml_node& ee, const std::string& att)
    {
        return fmt::format("{}@{}", get_path(ee), att);
    }

    void report_unused_attributes(std::vector<Error>* errors, const std::string& type_name,
                                  const pugi::xml_node& e, const std::set<std::string>& unused_values,
                                  const std::set<std::string>& available_names,
                                  const could_be_fun& could_be)
    {
        if (errors == nullptr)
        {
            return;
        }

        errors->emplace_back(
            fmt::format("Found unused attributes for type {} at {}:", type_name, get_path(e)));

        const auto values = std::vector<std::string>(available_names.begin(), available_names.end());

        for (const auto& unused : unused_values)
        {
            errors->emplace_back(fmt::format("Invalid attribute {} at {} and it could be {}", unused,
                                             get_path_of_attribute(e, unused),
                                             could_be(unused, values)));
        }
    }

    void report_unused_elements(std::vector<Error>* errors, const std::string& type_name,
                                const pugi::xml_node& e, const std::set<std::string>& unused_values,
                                const std::set<std::string>& available_names,
                                const could_be_fun& could_be)
    {
        if (errors == nullptr)
        {
            return;
        }

        errors->emplace_back(
            fmt::format("Found unused elements for type {} at {}:", type_name, get_path(e)));

        const auto values = std::vector<std::string>(available_names.begin(), available_names.end());

        for (const auto& unused : unused_values)
        {
            const auto c = could_be(unused, values);
            for (const auto& child : e.children(unused.c_str()))
            {
                errors->emplace_back(fmt::format("Invalid child {} at {} and it could be {}", unused,
                                                 get_path(child), c));
            }
        }
    }
}
