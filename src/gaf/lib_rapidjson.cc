#include "gaf/lib_rapidjson.h"

#include "fmt/format.h"

namespace gaf
{

    MissingType::MissingType(const std::string& t, const std::string& n)
        : type(t)
        , name(n)
    {
    }

    std::string GafToString(const rapidjson::Value& val)
    {
        if (val.IsNull())
        {
            return "Null";
        }
        else if (val.IsFalse())
        {
            return "False";
        }
        else if (val.IsTrue())
        {
            return "True";
        }
        else if (val.IsObject())
        {
            return "Object";
        }
        else if (val.IsArray())
        {
            return "Array";
        }
        else if (val.IsUint64())
        {
            std::stringstream ss;
            ss << "uint64 of " << val.GetUint64();
            return ss.str();
        }
        else if (val.IsInt64())
        {
            std::stringstream ss;
            ss << "int of " << val.GetInt64();
            return ss.str();
        }
        else if (val.IsFloat())
        {
            std::stringstream ss;
            ss << "float of " << val.GetFloat();
            return ss.str();
        }
        else if (val.IsString())
        {
            std::stringstream ss;
            ss << "string of " << val.GetString();
            return ss.str();
        }
        else
        {
            return "<unknown>";
        }
    }

    // todo(Gustav): remove this horrible function
    std::string GafToString(int64_t val)
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    std::vector<std::string> get_all_properties(const rapidjson::Value& e)
    {
        std::vector<std::string> r;

        if (e.IsObject() == false)
        {
            return r;
        }

        for (auto itr = e.MemberBegin(); itr != e.MemberEnd(); ++itr)
        {
            r.emplace_back(itr->name.GetString());
        }
        return r;
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


    void report_unused_struct
    (
        std::vector<Error>* errors,
        const std::string& struct_name,
        const rapidjson::Value& struct_source,
        const std::set<std::string>& unused_properties_set,
        const std::vector<MissingType>& missing_types,
        const std::vector<MissingType>& optional_types,
        const std::string& path,
        const could_be_fun& could_be
    )
    {
        if (errors == nullptr)
        {
            return;
        }

        if (struct_source.IsObject() == false)
        {
            errors->emplace_back(fmt::format("Expected object {} at {}:", struct_name, path));
            return;
        }

        if(missing_types.empty() == false)
        {
            // missing required proprty could be <list of unused properties>

            const std::vector<std::string> unused_properties{unused_properties_set.begin(), unused_properties_set.end()};
            for(const auto& m: missing_types)
            {
                errors->emplace_back(fmt::format(
                    "Missing a {} property named '{}' in object type {} at {}, could be {}",
                    m.type, m.name, struct_name, path,
                    could_be(m.name, unused_properties)
                ));
            }
        }
        else if (unused_properties_set.empty() == false)
        {
            // unused property, could be <list of optional properties>

            std::vector<std::string> optional_names;
            for(const auto& m: optional_types)
            {
                optional_names.emplace_back(m.name);
            }

            for(const auto& unused: unused_properties_set)
            {
                const auto found = struct_source.FindMember(unused.c_str());
                const auto json_type = GafToString(found->value);

                errors->emplace_back(fmt::format(
                    "Found unused proptery named '{1}' (a {0}) for type {2} at {3}.{1}: could be {4}",
                    json_type, unused, struct_name, path,
                    could_be(unused, optional_names)
                ));
            }
        }
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
