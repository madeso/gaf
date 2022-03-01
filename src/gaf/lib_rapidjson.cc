#include "gaf/lib_rapidjson.h"

#include "fmt/format.h"

namespace gaf
{
    std::string GafToString(const rapidjson::Value& val)
    {
        if (val.IsNull())
        {
            return "Null";
        };
        if (val.IsFalse())
        {
            return "False";
        };
        if (val.IsTrue())
        {
            return "True";
        };
        if (val.IsObject())
        {
            return "Object";
        };
        if (val.IsArray())
        {
            return "Array";
        };

        if (val.IsUint64())
        {
            std::stringstream ss;
            ss << "uint of " << val.GetUint64();
            return ss.str();
        }

        if (val.IsInt64())
        {
            std::stringstream ss;
            ss << "int of " << val.GetInt64();
            return ss.str();
        }

        if (val.IsFloat())
        {
            std::stringstream ss;
            ss << "double of " << val.GetFloat();
            return ss.str();
        }

        if (val.IsString())
        {
            std::stringstream ss;
            ss << "string of " << val.GetString();
            return ss.str();
        }

        return "<unknown>";
    }

    // todo(Gustav): remove this horrible function
    std::string GafToString(int64_t val)
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    std::set<std::string> get_all_properties_set(const rapidjson::Value& e)
    {
        std::set<std::string> r;

        if (e.IsObject() == false)
        {
            return r;
        }

        for (auto itr = e.MemberBegin(); itr != e.MemberEnd(); ++itr)
        {
            r.emplace(itr->name.GetString());
        }
        return r;
    }

    namespace
    {
        static const char* kTypeNames[] = {"Null",  "False",  "True",  "Object",
                                           "Array", "String", "Number"};
    }

    void report_unused(std::vector<Error>* errors, const std::string& type_name,
                       const std::string& path,
                       const rapidjson::Value& e, const std::set<std::string>& unused_values,
                                const std::set<std::string>& available_names,
                                const could_be_fun& could_be)
    {
        if (errors == nullptr)
        {
            return;
        }

        if (e.IsObject() == false)
        {
            errors->emplace_back(fmt::format("expected object {} at {}:", type_name, path));
            return;
        }

        errors->emplace_back(
            fmt::format("Found unused elements for type {} at {}:", type_name, path));

        const auto values = std::vector<std::string>(available_names.begin(), available_names.end());

        for (const auto& unused : unused_values)
        {
            const auto c = could_be(unused, values);

            for (auto itr = e.MemberBegin(); itr != e.MemberEnd();
                 ++itr)
            {
                errors->emplace_back(fmt::format("Unused {} named {} for {} and it could be {}",
                                                 kTypeNames[itr->value.GetType()],
                                                 itr->name.GetString(), unused, c));
            }
        }
    }
}
