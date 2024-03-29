#pragma once

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <sstream>
#include <limits>
#include <set>

#include "pugixml.hpp"

#include "gaf/lib_gaf.h"

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

    std::vector<std::string> get_all_attributes(const pugi::xml_node& e);
    std::vector<std::string> get_all_children(const pugi::xml_node& e);

    std::set<std::string> get_all_attributes_set(const pugi::xml_node& e);
    std::set<std::string> get_all_children_set(const pugi::xml_node& e);

    std::string get_path(const pugi::xml_node& e);

    void report_unused_attributes(std::vector<Error>* errors, const std::string& type_name,
                                  const pugi::xml_node& e, const std::set<std::string>& unused_values,
                                  const std::set<std::string>& available_names,
                                  const could_be_fun& could_be);
    void report_unused_elements(std::vector<Error>* errors, const std::string& type_name,
                                const pugi::xml_node& e, const std::set<std::string>& unused_values,
                                const std::set<std::string>& available_names,
                                const could_be_fun& could_be);
}
