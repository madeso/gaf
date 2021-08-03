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

    std::string vararg_json_return_error(const std::string& val, fmt::string_view format,
                                         fmt::format_args args)
    {
        return fmt::format(
            "{{"
            "gaf_ss.str(\"\");"
            "gaf_ss << \"{}, path: \" << gaf_path << \", value: \" << GafToString({});"
            "return gaf_ss.str();"
            "}}",
            fmt::vformat(format, args), val);
    }

    template <typename S, typename... Args>
    std::string json_return_error(const std::string& val, const S& format, Args&&... args)
    {
        return vararg_json_return_error(val, format, fmt::make_args_checked<Args...>(format, args...));
    }

    VarValue get_cpp_parse_from_rapidjson_helper_int(Out* sources, StandardType t,
                                                     const std::string& member, const std::string& name,
                                                     const std::string& json)
    {
        const auto rti = json_return_error(json, "read value for {} was not a integer", name);
        const auto rtl = json_return_error("gafv", "read value for {} was to low", name);
        const auto rth = json_return_error("gafv", "read value for {} was to high", name);
        sources->source.addf("if({}.IsInt64()==false) {}", json, rti);
        sources->source.addf("auto gafv = {}.GetInt64();", json);
        sources->source.addf("if(gafv < std::numeric_limits<{}>::min()) {}", get_cpp_type(t), rtl);
        sources->source.addf("if(gafv > std::numeric_limits<{}>::max()) {}", get_cpp_type(t), rth);
        const auto var = fmt::format("c->{}", member);
        const auto val = fmt::format("static_cast<{}>(gafv)", get_cpp_type(t));
        return VarValue{var, val};
    }

    VarValue get_cpp_parse_from_rapidjson_helper_float(Out* sources, const std::string& member,
                                                       const std::string& name, const std::string& json)
    {
        const auto err = json_return_error(json, "read value for {} was not a number", name);
        sources->source.addf("if({}.IsNumber()==false) {}", json, err);
        const auto var = fmt::format("c->{}", member);
        const auto val = fmt::format("{}.GetDouble()", json);
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
            const auto err = json_return_error(json, "read value for {} was not a integer", name);
            sources->source.addf("if({}.IsInt64()==false) {{ {} }}", json, err);
            const auto var = fmt::format("c->{}", member);
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
            const auto err = json_return_error(json, "read value for {} was not a bool", name);
            sources->source.addf("if({}.IsBool()==false) {{ {} }}", json, err);
            const auto var = fmt::format("c->{}", member);
            const auto val = fmt::format("{}.GetBool()", json);
            return VarValue{var, val};
        }
        case StandardType::String:
        {
            const auto err = json_return_error(json, "read value for {} was not a string", name);
            sources->source.addf("if({}.IsString()==false) {{ {} }}", json, err);
            const auto var = fmt::format("c->{}", member);
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
            const auto err =
                json_return_error("arr", "tried to read {} but value was not a array", name);
            sources->source.add("const rapidjson::Value& arr = iter->value;");
            sources->source.addf("if(!arr.IsArray()) {}", err);
            sources->source.add("for (rapidjson::SizeType i=0; i<arr.Size(); i++)");
            sources->source.add("{");
            const auto vv = get_cpp_parse_from_rapidjson_base(sources, t, member, name, "arr[i]");
            sources->source.addf("{}.push_back({});", vv.variable, vv.value);
            sources->source.add("}");
        }
        else
        {
            const auto vv = get_cpp_parse_from_rapidjson_base(sources, t, member, name, "iter->value");
            if (member_type.is_optional)
            {
                sources->source.addf("{} = std::make_shared<{}>({});", vv.variable, get_cpp_type(t),
                                     vv.value);
            }
            else
            {
                sources->source.addf("{} = {};", vv.variable, vv.value);
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
                const auto lines = make_array<std::string>(
                    "const rapidjson::Value& arr = iter->value;", "if(!arr.IsArray()) {err}",
                    "for (rapidjson::SizeType i=0; i<arr.Size(); i++)", "{{", "{type} temp;",
                    "gaf_ss.str(\"\");", "gaf_ss << gaf_path << \".{name}[\" << i << \"]\";",
                    "{rv} r = ReadFromJsonValue(&temp,arr[i], gaf_ss.str());",
                    "if(r{false}) {{ return r; }}", "c->{name}.push_back(temp);", "}}");
                for (const auto& line : lines)
                {
                    const auto err =
                        json_return_error("arr", "tried to read {} but value was not a array", m.name);
                    sources->source.add(
                        fmt::format(line, fmt::arg("name", m.name), fmt::arg("type", m.type_name.name),
                                    fmt::arg("err", err), fmt::arg("false", ".empty() == false"),
                                    fmt::arg("rv", "std::string")));
                }
            }
            else if (m.is_optional)
            {
                const auto lines = make_array<std::string>(
                    "c->{name} = std::make_shared<{type}>();"
                    "gaf_ss.str(\"\");"
                    "gaf_ss << gaf_path << \".{name}\";"
                    "{rv} r = ReadFromJsonValue(c->{name}.get(),iter->value, gaf_ss.str());"
                    "if(r{false})"
                    "{{"
                    "  c->{name}.reset();"
                    "  return r;"
                    "}}");
                for (const auto& line : lines)
                {
                    sources->source.add(fmt::format(
                        line, fmt::arg("name", m.name), fmt::arg("type", m.type_name.name),
                        fmt::arg("false", ".empty() == false"), fmt::arg("rv", "std::string")));
                }
            }
            else
            {
                const auto lines = make_array<std::string>(
                    "gaf_ss.str("
                    ");"
                    "gaf_ss << gaf_path << \".{name}\";"
                    "{rv} r = ReadFromJsonValue(&c->{name},iter->value, gaf_ss.str());"
                    "if(r{false})"
                    "{{"
                    "  return r;"
                    "}}");
                for (const auto& line : lines)
                {
                    sources->source.add(fmt::format(line, fmt::arg("name", m.name),
                                                    fmt::arg("false", ".empty() == false"),
                                                    fmt::arg("rv", "std::string")));
                }
            }
        }
    }

    void write_json_source_for_cpp(Out* sources, const Struct& s)
    {
        sources->source.addf(
            "{} ReadFromJsonValue({}* c, const rapidjson::Value& value, const std::string& gaf_path) {{",
            "std::string", s.name);
        sources->source.add("std::stringstream gaf_ss;");
        sources->source.addf(
            "if(!value.IsObject()) {}",
            json_return_error("value", "tried to read {} but value was not a object", s.name));
        sources->source.add("rapidjson::Value::ConstMemberIterator iter;");
        for (const auto& m : s.members)
        {
            sources->source.addf("iter = value.FindMember(\"{}\");", m.name);
            sources->source.add("if(iter != value.MemberEnd()) {");
            write_json_member(m, sources);
            sources->source.add("}");
            if (m.missing_is_fail || m.is_optional)
            {
                sources->source.add("else");
                sources->source.add("{");
                if (m.is_optional)
                {
                    sources->source.addf("c->{}.reset();", m.name);
                }
                else
                {
                    sources->source.add(json_return_error("value", "missing {} in json object", m.name));
                }
                sources->source.add("}");
            }
        }
        sources->source.addf("return {};", "\"\"");
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
        const auto arg = ", const std::string& gaf_path";
        sources->header.addf("{} ReadFromJsonValue({}* c, const rapidjson::Value& value{});",
                             "std::string", enum_type, arg);
        sources->source.addf("{} ReadFromJsonValue({}* c, const rapidjson::Value& value{})",
                             "std::string", enum_type, arg);
        sources->source.add("{");
        sources->source.add("std::stringstream gaf_ss;");
        sources->source.addf("if(value.IsString()==false) {};",
                             json_return_error("value", "read value for {} was not a string", e.name));
        for (const auto& v : e.values)
        {
            sources->source.add(
                fmt::format("if(strcmp(value.GetString(), \"{v}\")==0) {{ *c = {p}{v}; return {ok};}}",
                            fmt::arg("v", v), fmt::arg("p", value_prefix), fmt::arg("ok", "\"\"")));
        }
        sources->source.add(json_return_error("value", "read string for {} was not valid", e.name));
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
        sources.header.add("");
        sources.header.addf("#include \"gaf_{}.h\"", name);
        sources.source.add("#include <sstream>");
        sources.add("");

        if (f.package_name.empty() == false)
        {
            sources.addf("namespace {}", f.package_name);
            sources.add("{");
        }

        if (f.typedefs.empty() == false)
        {
            for (const auto& s : f.typedefs)
            {
                sources.header.addf("struct {};", s->name);
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

            sources.header.add("");
            const auto arg = ", const std::string& gaf_path";
            sources.header.addf("{} ReadFromJsonValue({}* c, const rapidjson::Value& value{});",
                                "std::string", s->name, arg);
            sources.header.add("");
        }

        sources.header.add("std::string GafToString(const rapidjson::Value& val);");
        sources.source.add("std::string GafToString(const rapidjson::Value& val)");
        sources.source.add("{");
        sources.source.add("if(val.IsNull()) { return \"Null\"; };");
        sources.source.add("if(val.IsFalse()) { return \"False\"; };");
        sources.source.add("if(val.IsTrue()) { return \"True\"; };");
        sources.source.add("if(val.IsObject()) { return \"Object\"; };");
        sources.source.add("if(val.IsArray()) { return \"Array\"; };");
        sources.source.add(
            "if(val.IsUint64()) { std::stringstream ss; ss << \"uint of \" << val.GetUint64(); return "
            "ss.str(); };");
        sources.source.add(
            "if(val.IsInt64()) { std::stringstream ss; ss << \"int of \" << val.GetInt64(); return "
            "ss.str(); };");
        sources.source.add(
            "if(val.IsDouble()) { std::stringstream ss; ss << \"double of \" << val.GetDouble(); return "
            "ss.str(); };");
        sources.source.add(
            "if(val.IsString()) { std::stringstream ss; ss << \"string of \" << val.GetString(); return "
            "ss.str(); };");
        sources.source.add("return \"<unknown>\";");
        sources.source.add("}");
        sources.source.add("");

        // todo: remove this horrible function
        sources.header.add("std::string GafToString(int64_t val);");
        sources.source.add("std::string GafToString(int64_t val)");
        sources.source.add("{");
        sources.source.add("std::stringstream ss;");
        sources.source.add("ss << val;");
        sources.source.add("return ss.str();");
        sources.source.add("}");

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
