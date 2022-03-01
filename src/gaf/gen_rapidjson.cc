#include "gaf/gen_rapidjson.h"

#include <array>
#include <cassert>
#include "fmt/format.h"

#include "gaf/generator.h"
#include "gaf/args.h"
#include "gaf/array.h"


#if 0
namespace json
{
    void add_enum_function(const Enum& e, Out* sources)
    {
        // const auto enum_type = e.name;
        // const auto value_prefix = get_value_prefix_opt(e);
        // const auto arg = ", const std::string& gaf_path";

        const auto signature = fmt::format(
            "std::string ParseEnumString({}* c, const char* value, const ::gaf::could_be_fun& could_be)",
            e.name);
        sources->header.addf("{};", signature);
        sources->source.add(signature);
        sources->source.add("{");
        for (const auto& v : e.values)
        {
            sources->source.addf(
                "if(strcmp(value, \"{value}\") == 0) {{ *c = {type}::{value}; return \"\"; }}",
                fmt::arg("type", e.name), fmt::arg("value", v));
        }
        sources->source.add("const auto all_values = std::vector<std::string>");
        sources->source.add("{");
        auto count = e.values.size();
        for (const auto& v : e.values)
        {
            count -= 1;
            if (count == 0)
            {
                sources->source.addf("\"{}\"", v);
            }
            else
            {
                sources->source.addf("\"{}\",", v);
            }
        }
        sources->source.add("};");
        sources->source.add("const auto cv = could_be(value, all_values);");
        sources->source.add("if(cv.empty())");
        sources->source.add("{");
        sources->source.addf("return fmt::format(\"{{}} is not a valid name for enum {}\", value);",
                             e.name);
        sources->source.add("}");
        sources->source.add("else");
        sources->source.add("{");
        sources->source.addf(
            "return fmt::format(\"{{}} is not a valid name for enum {}, could be {{}}\", value, cv);",
            e.name);
        sources->source.add("}");
        sources->source.add("}");
        sources->source.add("");
    }

    void on_xml_error(Out* sources, const std::string& error_exp)
    {
        sources->source.add("loaded_ok = false;");
        sources->source.add("if(errors != nullptr)");
        sources->source.add("{");
        sources->source.addf("errors->emplace_back({});", error_exp);
        sources->source.add("}");
    }

    void add_member_failure_to_read(Out* sources, const Member& m, const std::string& get_values)
    {
        if (m.missing_is_fail || m.is_optional)
        {
            sources->source.add("else");
            sources->source.add("{");
            if (m.is_optional)
            {
                sources->source.addf("c.{}.reset();", m.name);
            }
            else
            {
                sources->source.addf("const auto cv = could_be(\"{}\", {});", m.name, get_values);
                sources->source.addf("const auto path_to_val = path + \".{}\";", m.name);
                sources->source.add("if(cv.empty())");
                sources->source.add("{");
                on_xml_error(sources,
                             fmt::format("fmt::format(\"{} is missing, {{}}\", path_to_val)", m.name));
                sources->source.add("}");
                sources->source.add("else");
                sources->source.add("{");
                on_xml_error(
                    sources,
                    fmt::format("fmt::format(\"{} is missing, {{}}, could be {{}}\", path_to_val, cv)",
                                m.name));
                sources->source.add("}");
            }
            sources->source.add("}");
        }
    }

    bool is_basic_type(const Type& t)
    {
        switch (t.standard_type)
        {
        case StandardType::Int8:
        case StandardType::Int16:
        case StandardType::Int32:
        case StandardType::Int64:
        case StandardType::Uint8:
        case StandardType::Uint16:
        case StandardType::Uint32:
        case StandardType::Uint64:
        case StandardType::Float:
        case StandardType::Double:
        case StandardType::Bool:
        case StandardType::String:
            return true;
        default:
            return false;
        }
    }

    void read_variable_from(Out* sources, const std::string& var, const std::string& name, bool check)
    {
        if (check)
        {
            sources->source.addf("const auto found = {0}.find(\"{1}\");", var, name);
            sources->source.addf("if(found != {0}.end())", var);
            sources->source.add("{");
            sources->source.addf("{}.erase(found);", var);
            sources->source.add("}");
        }
        else
        {
            sources->source.addf("{0}.erase({0}.find(\"{1}\"));", var, name);
        }
    }

    void read_attribute(Out* sources, const std::string& name)
    {
        read_variable_from(sources, "list_of_attributes", name, false);
    }

    void read_single_node(Out* sources, const std::string& name)
    {
        read_variable_from(sources, "list_of_children", name, false);
    }

    void read_array_nodes(Out* sources, const std::string& name)
    {
        read_variable_from(sources, "list_of_children", name, true);
    }

    void add_member_variable_array(Out* sources, const Member& m)
    {
        if (m.type_name.is_enum)
        {
            sources->source.addf("children.emplace(\"{}\");", m.name);
            sources->source.addf("for(const auto el: value.children(\"{}\"))", m.name);
            sources->source.add("{");
            read_array_nodes(sources, m.name);
            sources->source.addf("auto em = {}{{}};", m.type_name.get_cpp_type());
            sources->source.add(
                "if(const auto error = ParseEnumString(&em, el.child_value(), could_be); error.empty() "
                "== "
                "false)");
            sources->source.add("{");
            on_xml_error(sources, "error");
            sources->source.add("}");
            sources->source.addf("c.{}.emplace_back(em);", m.name);
            sources->source.add("}");
        }
        else if (is_basic_type(m.type_name))
        {
            sources->source.addf("children.emplace(\"{}\");", m.name);
            sources->source.addf("for(const auto el: value.children(\"{}\"))", m.name);
            sources->source.add("{");
            read_array_nodes(sources, m.name);
            switch (m.type_name.standard_type)
            {
            case StandardType::Bool:
                sources->source.add("bool b = false;");
                sources->source.add("if(gaf::parse_bool(&b, el.child_value()) == false)");
                sources->source.add("{");
                on_xml_error(
                    sources,
                    fmt::format("fmt::format(\"Invalid bool for {}: {{}}\", el.child_value())", m.name));
                sources->source.add("}");
                sources->source.addf("c.{}.emplace_back(b);", m.name);
                break;
            case StandardType::String:
                sources->source.addf("c.{}.emplace_back(el.child_value());", m.name);
                break;
            default:
                sources->source.add("const auto property = el.child_value();");
                sources->source.addf("const auto parsed = ::gaf::parse_number<{}>(property);",
                                     m.type_name.get_cpp_type());
                sources->source.add("if(!parsed)");
                sources->source.add("{");
                on_xml_error(
                    sources,
                    fmt::format("fmt::format(\"Invalid format for {}: {{}}\", property)", m.name));
                sources->source.add("}");
                sources->source.addf("c.{}.emplace_back(*parsed);", m.name);
                break;
            }
            sources->source.add("}");
        }
        else
        {
            sources->source.addf("children.emplace(\"{}\");", m.name);
            sources->source.addf("for(const auto el: value.children(\"{}\"))", m.name);
            sources->source.add("{");
            read_array_nodes(sources, m.name);
            sources->source.addf("if(auto v = ReadJson{}(errors, el, could_be, path + \".{}[todo_i]\"); v)",
                                 m.type_name.name, m.name);
            sources->source.add("{");
            sources->source.addf("c.{}.emplace_back(*v);", m.name);
            sources->source.add("}");
            sources->source.add("else");
            sources->source.add("{");
            on_xml_error(
                sources,
                fmt::format("fmt::format(\"Failed to read {}: {{}}\", ::gaf::get_path(el))", m.name));
            sources->source.add("}");
            sources->source.add("}");
        }
    }

    void add_member_variable_single(Out* sources, const Member& m)
    {
        auto ptr = m.is_optional ? fmt::format("c.{}.get()", m.name) : fmt::format("&c.{}", m.name);
        auto val = m.is_optional ? fmt::format("*c.{}", m.name) : fmt::format("c.{}", m.name);
        auto create_mem = [sources, m]() {
            if (m.is_optional)
            {
                sources->source.addf("c.{} = std::make_shared<{}>();", m.name,
                                     m.type_name.get_cpp_type());
            }
        };
        auto clear_mem = [sources, m]() {
            if (m.is_optional)
            {
                sources->source.addf("c.{}.reset();", m.name);
            }
        };
        if (m.type_name.is_enum)
        {
            sources->source.addf("attributes.emplace(\"{}\");", m.name);
            sources->source.addf("if(const auto el = value.FindMember(\"{}\"); el != value.MemberEnd())", m.name);
            sources->source.add("{");
            read_attribute(sources, m.name);
            create_mem();
            sources->source.addf(
                "if(const auto error = ParseEnumString({}, el.value(), could_be); error.empty() == "
                "false)",
                ptr);
            sources->source.add("{");
            clear_mem();
            on_xml_error(sources, "error");
            sources->source.add("}");
            sources->source.add("}");
            add_member_failure_to_read(sources, m, "::gaf::get_all_attributes(value)");
        }
        else if (is_basic_type(m.type_name))
        {
            sources->source.addf("attributes.emplace(\"{}\");", m.name);
            sources->source.addf("if(const auto el = value.attribute(\"{}\"); el)", m.name);
            sources->source.add("{");
            read_attribute(sources, m.name);
            create_mem();
            switch (m.type_name.standard_type)
            {
            case StandardType::Bool:
                sources->source.addf("if(gaf::parse_bool({}, el.value()) == false)", ptr);
                sources->source.add("{");
                clear_mem();
                on_xml_error(
                    sources,
                    fmt::format("fmt::format(\"Invalid bool for {}: {{}}\", el.value())", m.name));
                sources->source.add("}");
                break;
            case StandardType::String:
                sources->source.addf("{} = el.value();", val);
                break;
            default:
                sources->source.add("const auto property = el.value();");
                sources->source.addf("const auto parsed = ::gaf::parse_number<{}>(property);",
                                     m.type_name.get_cpp_type());
                sources->source.add("if(parsed)");
                sources->source.add("{");
                sources->source.addf("{} = *parsed;", val);
                sources->source.add("}");
                sources->source.add("else");
                sources->source.add("{");
                clear_mem();
                on_xml_error(
                    sources,
                    fmt::format("fmt::format(\"Invalid format for {}: {{}}\", property)", m.name));
                sources->source.add("}");
                break;
            }
            sources->source.add("}");
            add_member_failure_to_read(sources, m, "::gaf::get_all_attributes(value)");
        }
        else
        {
            sources->source.addf("children.emplace(\"{}\");", m.name);
            sources->source.addf("if(const auto child = value.child(\"{}\"); child)", m.name);
            sources->source.add("{");
            read_single_node(sources, m.name);
            sources->source.addf("if(auto v = ReadJson{}(errors, child, could_be, path + \".{}\"); v)",
                                 m.type_name.name, m.name);
            sources->source.add("{");
            create_mem();
            sources->source.addf("{} = std::move(*v);", val);
            sources->source.add("}");
            sources->source.add("else");
            sources->source.add("{");
            clear_mem();
            on_xml_error(
                sources,
                fmt::format("fmt::format(\"Failed to read {}: {{}}\", ::gaf::get_path(child))", m.name));
            sources->source.add("}");
            sources->source.add("}");
            add_member_failure_to_read(sources, m, "::gaf::get_all_children(child)");
        }
    }

    void add_member_variable(Out* sources, const Member& m)
    {
        sources->source.add(std::string(80, '/'));
        sources->source.addf("// {}", m.name);
        if (m.is_dynamic_array)
        {
            add_member_variable_array(sources, m);
        }
        else
        {
            add_member_variable_single(sources, m);
        }
        sources->source.add("");
    }

    void add_unused_xml(Out* sources, const std::string& missing_values, bool is_attribute,
                        const std::string& existing_names, const Struct& s)
    {
        sources->source.addf("if({}.empty() == false)", missing_values);
        sources->source.add("{");
        sources->source.add("loaded_ok = false;");
        const auto function_name = is_attribute ? "report_unused_attributes" : "report_unused_elements";
        sources->source.addf("::gaf::{}(errors, \"{}\", value, {}, {}, could_be);", function_name,
                             s.name, missing_values, existing_names);
        sources->source.add("}");
    }

    void add_struct_function(Out* sources, const Struct& s)
    {
        const auto signature = fmt::format(
            "std::optional<{0}> ReadJson{0}(std::vector<::gaf::Error>* errors, const "
            "rapidjson::Value& value, [[maybe_unused]] const ::gaf::could_be_fun& could_be, const std::string& path) noexcept",
            s.name);
        sources->header.addf("{};", signature);
        sources->source.add(signature);
        sources->source.add("{");
        sources->source.add("if(value.IsObject() == false)");
        sources->source.add("{");
        on_xml_error(
            sources,
            fmt::format("fmt::format(\"Expected to read object {} but this is not a object {{}}\", path)", s.name));
        sources->source.add("return std::nullopt;");
        sources->source.add("}");
        sources->source.addf("{} c;", s.name);
        sources->source.add("bool loaded_ok = true;");
        sources->source.add("auto list_of_children = ::gaf::get_all_properties_set(value);");
        sources->source.add("auto children = std::set<std::string>();");
        sources->source.add("");
        for (const auto& m : s.members)
        {
            add_member_variable(sources, m);
        }
        add_unused_xml(sources, "list_of_children", false, "children", s);
        sources->source.add("");
        sources->source.add("if(loaded_ok)");
        sources->source.add("");
        sources->source.add("{");
        sources->source.add("return c;");
        sources->source.add("}");
        sources->source.add("else");
        sources->source.add("{");
        sources->source.add("return std::nullopt;");
        sources->source.add("}");
        sources->source.add("}");
        sources->source.add("");
    }

    Out generate_json(const File& f, const std::string& name)
    {
        auto sources = Out{};

        sources.header.add("#pragma once");
        sources.header.add("");
        sources.header.add("#include <string>");
        sources.header.add("#include <optional>");
        sources.header.add("#include <vector>");
        sources.header.add("#include \"rapidjson/document.h\"");
        sources.header.add("");
        sources.header.addf("#include \"gaf_{}.h\"", name);
        sources.header.add("#include \"gaf/lib_rapidjson.h\"");

        sources.source.add("#include <cstring>");
        sources.source.add("#include <sstream>");
        sources.source.add("#include \"fmt/format.h\"");

        sources.add("");

        if (f.package_name.empty() == false)
        {
            sources.addf("namespace {}", f.package_name);
            sources.add("{");
        }

        for (const auto& e : f.enums)
        {
            add_enum_function(*e, &sources);
        }

        for (const auto& s : f.structs)
        {
            add_struct_function(&sources, *s);
        }

        if (f.package_name.empty() == false)
        {
            sources.add("}");
            sources.add("");
        }

        return sources;
    }
}

#else

namespace json
{
    struct VarValue
    {
        std::string variable;
        std::string value;
    };

    void vararg_json_return_error(Lines* lines, const std::string& val, fmt::string_view format,
                                         fmt::format_args args)
    {
        // need extra {} as it's used as a statement
        lines->add("{");
        lines->add("if(errors != nullptr)");
        lines->add("{");
        lines->add("gaf_ss.str(\"\");");
        lines->addf("gaf_ss << \"{}, path: \" << gaf_path << \", value: \" << ::gaf::GafToString({});", fmt::vformat(format, args), val);
        lines->add("errors->emplace_back(gaf_ss.str());");
        lines->add("}");
        lines->add("return std::nullopt;");
        lines->add("}");
    }

    template <typename S, typename... Args>
    void json_return_error(Lines* lines, const std::string& val, const S& format, Args&&... args)
    {
        vararg_json_return_error(lines, val, format, fmt::make_args_checked<Args...>(format, args...));
    }

    VarValue get_cpp_parse_from_rapidjson_helper_int(Out* sources, StandardType t,
                                                     const std::string& member, const std::string& name,
                                                     const std::string& json)
    {
        sources->source.addf("if({}.IsInt64()==false)", json);
        json_return_error(&sources->source, json, "read value for {} was not a integer", name);

        sources->source.addf("auto gafv = {}.GetInt64();", json);
        sources->source.addf("if(gafv < std::numeric_limits<{}>::min())", get_cpp_type(t));
        json_return_error(&sources->source, "gafv", "read value for {} was to low", name);
        sources->source.addf("if(gafv > std::numeric_limits<{}>::max())", get_cpp_type(t));
        json_return_error(&sources->source, "gafv", "read value for {} was to high", name);
        const auto var = fmt::format("ret.{}", member);
        const auto val = fmt::format("static_cast<{}>(gafv)", get_cpp_type(t));
        return VarValue{var, val};
    }

    VarValue get_cpp_parse_from_rapidjson_helper_float(Out* sources, const std::string& member,
                                                       const std::string& name, const std::string& json)
    {
        sources->source.addf("if({}.IsNumber()==false)", json);
        json_return_error(&sources->source, json, "read value for {} was not a number", name);
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
            sources->source.addf("if({}.IsInt64()==false)", json);
            json_return_error(&sources->source, json, "read value for {} was not a integer", name);
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
            sources->source.addf("if({}.IsBool()==false)", json);
            json_return_error(&sources->source, json, "read value for {} was not a bool", name);
            const auto var = fmt::format("ret.{}", member);
            const auto val = fmt::format("{}.GetBool()", json);
            return VarValue{var, val};
        }
        case StandardType::String:
        {
            sources->source.addf("if({}.IsString()==false)", json);
            json_return_error(&sources->source, json, "read value for {} was not a string", name);
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
            sources->source.addf("if(!arr.IsArray())");
            json_return_error(&sources->source, "arr", "tried to read {} but value was not a array", name);
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
                sources->source.add("const rapidjson::Value& arr = iter->value;");
                sources->source.add("if(!arr.IsArray())");
                json_return_error(&sources->source, "arr", "tried to read {} but value was not a array", m.name);
                const auto lines = make_array<std::string>
                (
                    "for (rapidjson::SizeType i=0; i<arr.Size(); i++)",
                    "{{",
                    "gaf_ss.str(\"\");",
                    "gaf_ss << gaf_path << \".{name}[\" << i << \"]\";",
                    "auto temp = ReadJson{type}(errors, arr[i], could_be, gaf_ss.str());",
                    "if(temp.has_value() == false)",
                    "{{",
                    "gaf_ok = false;",
                    "}}",
                    "else",
                    "{{",
                    "ret.{name}.push_back(std::move(*temp));",
                    "}}",
                    "}}"
                );
                for (const auto& line : lines)
                {
                    sources->source.add(fmt::format(line,
                        fmt::arg("name", m.name),
                        fmt::arg("type", m.type_name.name),
                        fmt::arg("false", ".empty() == false"),
                        fmt::arg("rv", "std::string")
                    ));
                }
            }
            else if (m.is_optional)
            {
                const auto lines = make_array<std::string>(
                    "gaf_ss.str(\"\");",
                    "gaf_ss << gaf_path << \".{name}\";",
                    "auto temp = ReadJson{type}(errors, iter->value, could_be, gaf_ss.str());",
                    "if(temp.has_value() == false)",
                    "{{",
                    "ret.{name}.reset();",
                    "gaf_ok = false;"
                    "}}",
                    "else",
                    "{{",
                    "ret.{name} = std::make_shared<{type}>(std::move(*temp));",
                    "}}"
                );
                for (const auto& line : lines)
                {
                    sources->source.add(fmt::format(line,
                        fmt::arg("name", m.name),
                        fmt::arg("type", m.type_name.name),
                        fmt::arg("false", ".empty() == false"),
                        fmt::arg("rv", "std::string")
                    ));
                }
            }
            else
            {
                const auto lines = make_array<std::string>(
                    "gaf_ss.str(\"\");",
                    "gaf_ss << gaf_path << \".{name}\";",
                    "auto temp = ReadJson{type}(errors, iter->value, could_be, gaf_ss.str());",
                    "if(temp.has_value() == false)",
                    "{{",
                    "gaf_ok = false;",
                    "}}",
                    "else",
                    "{{",
                    "ret.{name} = std::move(*temp);",
                    "}}"
                );
                for (const auto& line : lines)
                {
                    sources->source.add(fmt::format(line,
                        fmt::arg("name", m.name),
                        fmt::arg("type", m.type_name.name),
                        fmt::arg("false", ".empty() == false"),
                        fmt::arg("rv", "std::string")
                    ));
                }
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
        sources->source.addf("{}{{",signature);
        sources->header.addf("{};", signature);

        sources->source.add("std::stringstream gaf_ss;");
        sources->source.add("if(!value.IsObject())");
        json_return_error(&sources->source, "value", "tried to read {} but value was not a object", s.name);
        sources->source.add("");
        sources->source.add("bool gaf_ok = true;");
        sources->source.addf("{} ret;", s.name);
        sources->source.add("");
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
                    sources->source.addf("ret.{}.reset();", m.name);
                }
                else
                {
                    json_return_error(&sources->source, "value", "missing {} in json object", m.name);
                }
                sources->source.add("}");
            }
        }
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
        sources->header.addf("{};", signature);

        sources->source.add("{");
        sources->source.add("std::stringstream gaf_ss;");
        sources->source.add("if(value.IsString()==false)");
        json_return_error(&sources->source, "value", "read value for {} was not a string", e.name);
        for (const auto& v : e.values)
        {
            sources->source.add(
                fmt::format("if(strcmp(value.GetString(), \"{v}\")==0) {{ return {p}{v};}}",
                            fmt::arg("v", v), fmt::arg("p", value_prefix)));
        }
        json_return_error(&sources->source, "value", "read string for {} was not valid", e.name);
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

#endif


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
