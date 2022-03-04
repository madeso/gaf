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
        return GafToString(val, 0);
    }

    std::string GafToString(const rapidjson::Value& val, int level)
    {
        if (val.IsNull())
        {
            return "null";
        }
        else if (val.IsFalse())
        {
            return "false";
        }
        else if (val.IsTrue())
        {
            return "true";
        }
        else if (val.IsObject())
        {
            if(level < 3)
            {
                if(const auto id = val.FindMember("id"); id != val.MemberEnd())
                {
                    return fmt::format("object with id={}", GafToString(id->value, level+1));
                }
                else if(const auto name = val.FindMember("name"); name != val.MemberEnd())
                {
                    return fmt::format("object with name={}", GafToString(name->value, level+1));
                }
            }
            return "object";
        }
        else if (val.IsArray())
        {
            return "Array";
        }
        else if (val.IsUint64())
        {
            return fmt::format("{} (uint64)", val.GetUint64());
        }
        else if (val.IsInt64())
        {
            return fmt::format("{} (int)", val.GetInt64());
        }
        else if (val.IsFloat())
        {
            return fmt::format("{} (float)", val.GetFloat());
        }
        else if (val.IsString())
        {
            return fmt::format("'{}' (string)", val.GetString());
        }
        else
        {
            return "<unknown>";
        }
    }

    // todo(Gustav): remove this horrible function
    std::string GafToString(int64_t val)
    {
        return fmt::format("{}", val);
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


    void report_unused_struct
    (
        std::vector<Error>* errors,
        const std::string& struct_name,
        const rapidjson::Value& struct_source,
        const std::set<std::string>& unused_properties_set,
        const std::vector<MissingType>& missing_types,
        const std::vector<MissingType>& optional_types,
        const std::vector<std::string>& all_struct_members,
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

                const auto could_be_optional = could_be(unused, optional_names);
                const auto could_be_all = could_be(unused, all_struct_members);

                // todo(Gustav): handle empty suggestions
                const auto could_be_mes = could_be_optional == could_be_all
                    ? fmt::format("could be {}", could_be_optional)
                    : fmt::format
                    (
                        "could be {0} but only {1} {2} specified",
                        could_be_all,
                        could_be_optional,
                        optional_names.size() == 1 ? "isn't" : "aren't"
                    )
                    ;

                errors->emplace_back(fmt::format(
                    "Found unused property in a {5} named '{1}': {0} for type {2}, at {3}.{1}: {4}",
                    json_type, unused, struct_name, path,
                    could_be_mes, GafToString(struct_source)
                ));
            }
        }
    }
}
