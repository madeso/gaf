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
    struct MissingType
    {
        std::string type;
        std::string name;

        MissingType(const std::string& t, const std::string& n);
    };


    std::string GafToString(const rapidjson::Value& val);
    std::string GafToString(int64_t val);

    std::vector<std::string> get_all_properties(const rapidjson::Value& e);
    std::set<std::string> get_all_properties_set(const rapidjson::Value& e);

    void report_unused_struct
    (
        std::vector<Error>* errors,
        const std::string& struct_name,
        const rapidjson::Value& struct_source,
        const std::set<std::string>& unused_properties,
        const std::vector<MissingType>& missing_types,
        const std::vector<MissingType>& optional_types,
        const std::string& path,
        const could_be_fun& could_be
    );
}
