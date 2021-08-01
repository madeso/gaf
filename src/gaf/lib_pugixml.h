#pragma once

#include <string>

namespace gaf
{
    // return true of ok, false if not
    bool parse_bool(bool* dest, const std::string& value);
}
