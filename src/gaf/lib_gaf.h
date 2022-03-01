#pragma once

#include <string>
#include <vector>
#include <functional>

namespace gaf
{
    struct Error
    {
        std::string description;

        explicit Error(const std::string& d);
    };

    using could_be_fun = std::function<std::string(const std::string&, const std::vector<std::string>&)>;

    std::string could_be_fun_none(const std::string& name, const std::vector<std::string>& values);
    std::string could_be_fun_all(const std::string& name, const std::vector<std::string>& values);
}
