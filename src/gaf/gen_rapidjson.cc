#include "gaf/gen_rapidjson.h"

#include <array>
#include <cassert>
#include "fmt/format.h"

#include "gaf/generator.h"
#include "gaf/args.h"
#include "gaf/array.h"

namespace json
{
    struct VarValue
    {
        std::string variable;
        std::string value;
    };

    void json_return_error(Lines* lines, const std::string& val, const std::string& message)
    {
        // need extra {} as it's used as a statement
        lines->add("{");
        lines->add("if(errors != nullptr)");
        lines->add("{");
        lines->add("gaf_ss.str(\"\");");
        lines->add(fmt::format("gaf_ss << \"{}, path: \" << gaf_path << \", got: \" << ::gaf::GafToString({});", message, val));
        lines->add("errors->emplace_back(gaf_ss.str());");
        lines->add("}");
        lines->add("return std::nullopt;");
        lines->add("}");
    }

    VarValue get_cpp_parse_from_rapidjson_helper_int(Out* sources, StandardType t,
                                                     const std::string& member, const std::string& name,
                                                     const std::string& json)
    {
        sources->source.add(fmt::format("if({}.IsInt64()==false)", json));
        json_return_error(&sources->source, json, fmt::format("read value for {} was not a integer", name));

        sources->source.add(fmt::format("auto gafv = {}.GetInt64();", json));
        sources->source.add(fmt::format("if(gafv < std::numeric_limits<{}>::min())", get_cpp_type(t)));
        json_return_error(&sources->source, "gafv", fmt::format("read value for {} was to low", name));
        sources->source.add(fmt::format("if(gafv > std::numeric_limits<{}>::max())", get_cpp_type(t)));
        json_return_error(&sources->source, "gafv", fmt::format("read value for {} was to high", name));
        const auto var = fmt::format("ret.{}", member);
        const auto val = fmt::format("static_cast<{}>(gafv)", get_cpp_type(t));
        return VarValue{var, val};
    }

    VarValue get_cpp_parse_from_rapidjson_helper_float(Out* sources, const std::string& member,
                                                       const std::string& name, const std::string& json)
    {
        sources->source.add(fmt::format("if({}.IsNumber()==false)", json));
        json_return_error(&sources->source, json, fmt::format("read value for {} was not a number", name));
        const auto var = fmt::format("ret.{}", member);
        const auto val = fmt::format("{}.GetFloat()", json);
        return VarValue{var, val};
    }

    VarValue get_cpp_parse_from_rapidjson_base(Out* sources, StandardType t, const std::string& member,
                                               const std::string& name, const std::string& json)
    {
        switch (t)
        {
        // todo: verify that all int parsing ranges are correct
        case StandardType::Int8:
            return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, name, json);
        case StandardType::Int16:
            return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, name, json);
        case StandardType::Int32:
            return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, name, json);
        case StandardType::Int64:
        {
            sources->source.add(fmt::format("if({}.IsInt64()==false)", json));
            json_return_error(&sources->source, json, fmt::format("read value for {} was not a integer", name));
            const auto var = fmt::format("ret.{}", member);
            const auto val = fmt::format("{}.GetInt64()", json);
            return VarValue{var, val};
        }
        case StandardType::Uint8:
            return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, name, json);
        case StandardType::Uint16:
            return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, name, json);
        case StandardType::Uint32:
            return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, name, json);
        case StandardType::Uint64:
            return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, name, json);
        case StandardType::Float:
            return get_cpp_parse_from_rapidjson_helper_float(sources, member, name, json);
        case StandardType::Double:
            return get_cpp_parse_from_rapidjson_helper_float(sources, member, name, json);
        case StandardType::Bool:
        {
            sources->source.add(fmt::format("if({}.IsBool()==false)", json));
            json_return_error(&sources->source, json, fmt::format("read value for {} was not a bool", name));
            const auto var = fmt::format("ret.{}", member);
            const auto val = fmt::format("{}.GetBool()", json);
            return VarValue{var, val};
        }
        case StandardType::String:
        {
            sources->source.add(fmt::format("if({}.IsString()==false)", json));
            json_return_error(&sources->source, json, fmt::format("read value for {} was not a string", name));
            const auto var = fmt::format("ret.{}", member);
            const auto val = fmt::format("{}.GetString()", json);
            return VarValue{var, val};
        }
        default:
            assert(false && "BUG: No type specified");
            return VarValue{member, "bug_unhandled_std_type"};
        }
    }

    void get_cpp_parse_from_rapidjson(Out* sources, const StandardType& t, const std::string& member,
                                      const std::string& name, const Member& member_type)
    {
        if (member_type.is_dynamic_array)
        {
            sources->source.add("const rapidjson::Value& arr = iter->value;");
            sources->source.add(fmt::format("if(!arr.IsArray())"));
            json_return_error(&sources->source, "arr", fmt::format("tried to read {} but value was not a array", name));
            sources->source.add("for (rapidjson::SizeType i=0; i<arr.Size(); i++)");
            sources->source.add("{");
            const auto vv = get_cpp_parse_from_rapidjson_base(sources, t, member, name, "arr[i]");
            sources->source.add(fmt::format("{}.push_back({});", vv.variable, vv.value));
            sources->source.add("}");
        }
        else
        {
            const auto vv = get_cpp_parse_from_rapidjson_base(sources, t, member, name, "iter->value");
            if (member_type.is_optional)
            {
                sources->source.add(fmt::format("{} = std::make_shared<{}>({});", vv.variable, get_cpp_type(t),
                                     vv.value));
            }
            else
            {
                sources->source.add(fmt::format("{} = {};", vv.variable, vv.value));
            }
        }
    }

    void write_json_member(const Member& m, Out* sources)
    {
        if (m.type_name.standard_type != StandardType::INVALID)
        {
            get_cpp_parse_from_rapidjson(sources, m.type_name.standard_type, m.name, m.name, m);
        }
        else
        {
            if (m.is_dynamic_array)
            {
                sources->source.add("const rapidjson::Value& arr = iter->value;");
                sources->source.add("if(!arr.IsArray())");
                json_return_error(&sources->source, "arr", fmt::format("tried to read {} but value was not a array", m.name));
                
                #define ADD_SOURCE(line) sources->source.add(fmt::format(line,\
                        fmt::arg("name", m.name),\
                        fmt::arg("type", m.type_name.name),\
                        fmt::arg("false", ".empty() == false"),\
                        fmt::arg("rv", "std::string")\
                    ))
                ADD_SOURCE("for (rapidjson::SizeType i=0; i<arr.Size(); i++)");
                ADD_SOURCE("{{");
                ADD_SOURCE("gaf_ss.str(\"\");");
                ADD_SOURCE("gaf_ss << gaf_path << \".{name}[\" << i << \"]\";");
                ADD_SOURCE("auto temp = ReadJson{type}(errors, arr[i], could_be, gaf_ss.str());");
                ADD_SOURCE("if(temp.has_value() == false)");
                ADD_SOURCE("{{");
                ADD_SOURCE("gaf_ok = false;");
                ADD_SOURCE("}}");
                ADD_SOURCE("else");
                ADD_SOURCE("{{");
                ADD_SOURCE("ret.{name}.push_back(std::move(*temp));");
                ADD_SOURCE("}}");
                ADD_SOURCE("}}");
                #undef ADD_SOURCE
            }
            else if (m.is_optional)
            {
                #define ADD_SOURCE(line) sources->source.add(fmt::format(line,\
                        fmt::arg("name", m.name),\
                        fmt::arg("type", m.type_name.name),\
                        fmt::arg("false", ".empty() == false"),\
                        fmt::arg("rv", "std::string")\
                    ))
                
                ADD_SOURCE("gaf_ss.str(\"\");");
                ADD_SOURCE("gaf_ss << gaf_path << \".{name}\";");
                ADD_SOURCE("auto temp = ReadJson{type}(errors, iter->value, could_be, gaf_ss.str());");
                ADD_SOURCE("if(temp.has_value() == false)");
                ADD_SOURCE("{{");
                ADD_SOURCE("ret.{name}.reset();");
                ADD_SOURCE("gaf_ok = false;");
                ADD_SOURCE("}}");
                ADD_SOURCE("else");
                ADD_SOURCE("{{");
                ADD_SOURCE("ret.{name} = std::make_shared<{type}>(std::move(*temp));");
                ADD_SOURCE("}}");
                #undef ADD_SOURCE
            }
            else
            {
                #define ADD_SOURCE(line) sources->source.add(fmt::format(line,\
                        fmt::arg("name", m.name),\
                        fmt::arg("type", m.type_name.name),\
                        fmt::arg("false", ".empty() == false"),\
                        fmt::arg("rv", "std::string")\
                    ))
                ADD_SOURCE("gaf_ss.str(\"\");");
                ADD_SOURCE("gaf_ss << gaf_path << \".{name}\";");
                ADD_SOURCE("auto temp = ReadJson{type}(errors, iter->value, could_be, gaf_ss.str());");
                ADD_SOURCE("if(temp.has_value() == false)");
                ADD_SOURCE("{{");
                ADD_SOURCE("gaf_ok = false;");
                ADD_SOURCE("}}");
                ADD_SOURCE("else");
                ADD_SOURCE("{{");
                ADD_SOURCE("ret.{name} = std::move(*temp);");
                ADD_SOURCE("}}");
                #undef ADD_SOURCE
            }
        }
    }

    void write_json_source_for_cpp(Out* sources, const Struct& s)
    {
        const auto signature = fmt::format(
            "std::optional<{0}> ReadJson{0}(std::vector<::gaf::Error>* errors, const "
            "rapidjson::Value& value, [[maybe_unused]] const ::gaf::could_be_fun& could_be, const "
            "std::string& gaf_path) noexcept",
            s.name);
        sources->header.add(fmt::format("{};", signature));
        sources->source.add(signature);
        sources->source.add("{");

        sources->source.add("auto unused_properties = ::gaf::get_all_properties_set(value);");
        sources->source.add("std::vector<::gaf::MissingType> missing_types;");
        sources->source.add("std::vector<::gaf::MissingType> optional_types;");
        sources->source.add("std::stringstream gaf_ss;"); // todo(Gustav): reduce scope?
        sources->source.add("");

        sources->source.add("if(!value.IsObject())");
        json_return_error(&sources->source, "value", fmt::format("tried to read {} but value was not a object", s.name));
        sources->source.add("");
        sources->source.add("bool gaf_ok = true;");
        sources->source.add(fmt::format("{} ret;", s.name));
        sources->source.add("");
        sources->source.add("rapidjson::Value::ConstMemberIterator iter;");
        for (const auto& m : s.members)
        {
            sources->source.add(fmt::format("iter = value.FindMember(\"{}\");", m.name));
            sources->source.add("if(iter != value.MemberEnd())");
            
            sources->source.add("{");
            write_json_member(m, sources);
            sources->source.add(fmt::format("unused_properties.erase(\"{}\");", m.name));
            sources->source.add("}");
            if (m.missing_is_fail || m.is_optional)
            {
                sources->source.add("else");
                sources->source.add("{");
                if (m.is_optional)
                {
                    sources->source.add(fmt::format("ret.{}.reset();", m.name));
                    sources->source.add(fmt::format
                    (
                        "optional_types.emplace_back(\"{}\", \"{}\");",
                        m.type_name.name, m.name
                    ));
                }
                else
                {
                    sources->source.add("gaf_ok = false;");
                    sources->source.add(fmt::format
                    (
                        "missing_types.emplace_back(\"{}\", \"{}\");",
                        m.type_name.name, m.name
                    ));
                }
                sources->source.add("}");
            }
        }
        
        sources->source.add("");
        sources->source.add("const std::vector<std::string> all_members = ");
        sources->source.add("{");
        auto count = s.members.size();
        for (const auto& m : s.members)
        {
            count -= 1;
            if (count == 0)
            {
                sources->source.add(fmt::format("\"{}\"", m.name));
            }
            else
            {
                sources->source.add(fmt::format("\"{}\",", m.name));
            }
        }
        sources->source.add("};");

        sources->source.add(fmt::format
        (
            "::gaf::report_unused_struct"
            "("
                "errors, "
                "\"{}\", "
                "value, "
                "unused_properties, "
                "missing_types, "
                "optional_types, "
                "all_members, "
                "gaf_path, "
                "could_be"
            ");"
            , s.name
        ));
        sources->source.add("if(gaf_ok == false) { return std::nullopt; }");
        sources->source.add("return ret;");
        sources->source.add("}");
        sources->source.add("");
    }

    std::string get_value_prefix_opt(const Enum& e)
    {
        return fmt::format("{}::", e.name);
    }

    void add_enum_json_function(const Enum& e, Out* sources, bool type_enum = false)
    {
        const auto enum_type = type_enum ? fmt::format("{}::Type", e.name) : e.name;
        const auto value_prefix = get_value_prefix_opt(e);
        
        const auto signature = fmt::format(
            "std::optional<{1}> ReadJson{0}(std::vector<::gaf::Error>* errors, const "
            "rapidjson::Value& value, [[maybe_unused]] const ::gaf::could_be_fun& could_be, const "
            "std::string& gaf_path) noexcept",
            e.name, enum_type);
        sources->source.add(signature);
        sources->header.add(fmt::format("{};", signature));

        sources->source.add("{");
        sources->source.add("std::stringstream gaf_ss;");
        sources->source.add("if(value.IsString()==false)");
        json_return_error(&sources->source, "value", fmt::format("read value for {} was not a string", e.name));
        for (const auto& v : e.values)
        {
            sources->source.add(
                fmt::format("if(strcmp(value.GetString(), \"{v}\")==0) {{ return {p}{v};}}",
                            fmt::arg("v", v), fmt::arg("p", value_prefix)));
        }
        sources->source.add("");
        sources->source.add("const auto all_values = std::vector<std::string>");
        sources->source.add("{");
        auto count = e.values.size();
        for (const auto& v : e.values)
        {
            count -= 1;
            if (count == 0)
            {
                sources->source.add(fmt::format("\"{}\"", v));
            }
            else
            {
                sources->source.add(fmt::format("\"{}\",", v));
            }
        }
        sources->source.add("};");
        sources->source.add("const auto cb = could_be(value.GetString(), all_values);");

        json_return_error(&sources->source, "value", fmt::format("'\"<<value.GetString() << \"' is not a valid value for enum '{}', could be \" << cb << \"", e.name));
        sources->source.add("}");
        sources->source.add("");
    }

    Out generate_json(const File& f, const std::string& name)
    {
        auto sources = Out{};

        sources.header.add("#pragma once");
        sources.header.add("");

        sources.header.add("#include <string>");
        sources.source.add("#include <cstring>");
        sources.header.add("#include \"rapidjson/document.h\"");
        sources.header.add("#include \"gaf/lib_rapidjson.h\"");
        sources.header.add("");
        sources.header.add(fmt::format("#include \"gaf_{}.h\"", name));
        sources.source.add("#include <sstream>");
        sources.add("");

        if (f.package_name.empty() == false)
        {
            sources.add(fmt::format("namespace {}", f.package_name));
            sources.add("{");
        }

        if (f.typedefs.empty() == false)
        {
            for (const auto& s : f.typedefs)
            {
                sources.header.add(fmt::format("struct {};", s->name));
            }
            sources.header.add("");
        }

        for (const auto& e : f.enums)
        {
            add_enum_json_function(*e, &sources);
        }

        for (const auto& s : f.structs)
        {
            write_json_source_for_cpp(&sources, *s);
        }


        sources.source.add("");

        if (f.package_name.empty() == false)
        {
            sources.add("}");
            sources.add("");
        }

        return sources;
    }

}


std::string RapidJsonPlugin::get_name()
{
    return "rapidjson";
}

int RapidJsonPlugin::run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args,
                                const std::string& name)
{
    if (auto r = no_arguments(args); r != 0)
    {
        return r;
    }

    auto out = json::generate_json(file, name);
    write_cpp(&out, writer, output_folder, name, "gaf_rapidjson_");

    return 0;
}
