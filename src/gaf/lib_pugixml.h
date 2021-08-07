#pragma once

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <sstream>
#include <limits>
#include <set>

#include "pugixml.hpp"

namespace gaf
{
    // return true of ok, false if not
    bool parse_bool(bool* dest, const std::string& value);

    template <typename T>
    ::std::optional<T> parse_number(const std::string& value)
    {
        T t = 0;

        std::istringstream ss(value);
        ss >> t;

        const auto is_valid = ss.eof() == true && ss.fail() == false;

        if (is_valid)
        {
            return t;
        }
        else
        {
            return {};
        }
    }

    template <typename T, typename TFallback>
    std::optional<T> cast_parse_number(const std::string& value)
    {
        const auto r = parse_number<TFallback>(value);
        if (r)
        {
            if (*r > static_cast<TFallback>(std::numeric_limits<T>::max()))
            {
                return {};
            }
            if (*r < static_cast<TFallback>(std::numeric_limits<T>::min()))
            {
                return {};
            }

            return static_cast<T>(*r);
        }
        else
        {
            return {};
        }
    }

    template <>
    std::optional<std::int8_t> parse_number<std::int8_t>(const std::string& value);

    template <>
    std::optional<std::uint8_t> parse_number<std::uint8_t>(const std::string& value);

    using could_be_fun = std::function<std::string(const std::string&, const std::vector<std::string>&)>;

    std::string could_be_fun_none(const std::string& name, const std::vector<std::string>& values);
    std::string could_be_fun_all(const std::string& name, const std::vector<std::string>& values);

    std::vector<std::string> get_all_attributes(const pugi::xml_node& e);
    std::vector<std::string> get_all_children(const pugi::xml_node& e);

    std::set<std::string> get_all_attributes_set(const pugi::xml_node& e);
    std::set<std::string> get_all_children_set(const pugi::xml_node& e);

    std::string get_path(const pugi::xml_node& e);
}
