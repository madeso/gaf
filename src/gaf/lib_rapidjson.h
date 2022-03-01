#pragma once

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <sstream>
#include <limits>
#include <set>

#include "rapidjson/document.h"

#include "gaf/lib_gaf.h"


namespace gaf
{
    std::string GafToString(const rapidjson::Value& val);
    std::string GafToString(int64_t val);

    std::set<std::string> get_all_properties_set(const rapidjson::Value& e);

    void report_unused(std::vector<Error>* errors, const std::string& type_name,
                       const rapidjson::Value& e, const std::set<std::string>& unused_values,
                                const std::set<std::string>& available_names,
                                const could_be_fun& could_be);
}
