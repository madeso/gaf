#include "gaf/gen_pugixml.h"

#include <array>
#include <cassert>
#include "fmt/format.h"

#include "gaf/generator.h"
#include "gaf/args.h"
#include "gaf/array.h"

namespace xml
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
                sources->source.add("const auto path_to_val = ::gaf::get_path(value);");
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
            sources->source.addf("if(auto v = ReadXmlElement{}(errors, el, could_be); v)",
                                 m.type_name.name);
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
            sources->source.addf("if(const auto el = value.attribute(\"{}\"); el)", m.name);
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
            sources->source.addf("if(auto v = ReadXmlElement{}(errors, child, could_be); v)",
                                 m.type_name.name);
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
            "std::optional<{0}> ReadXmlElement{0}(std::vector<::gaf::Error>* errors, const "
            "pugi::xml_node& value, [[maybe_unused]] const ::gaf::could_be_fun& could_be) noexcept",
            s.name);
        sources->header.addf("{};", signature);
        sources->source.add(signature);
        sources->source.add("{");
        sources->source.addf("{} c;", s.name);
        sources->source.add("bool loaded_ok = true;");
        sources->source.add("auto list_of_children = ::gaf::get_all_children_set(value);");
        sources->source.add("auto list_of_attributes = ::gaf::get_all_attributes_set(value);");
        sources->source.add("auto children = std::set<std::string>();");
        sources->source.add("auto attributes = std::set<std::string>();");
        sources->source.add("");
        for (const auto& m : s.members)
        {
            add_member_variable(sources, m);
        }
        add_unused_xml(sources, "list_of_children", false, "children", s);
        add_unused_xml(sources, "list_of_attributes", true, "attributes", s);
        sources->source.add("");

        sources->source.add("if(loaded_ok)");
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

    Out generate_xml(const File& f, const std::string& name)
    {
        auto sources = Out{};

        sources.header.add("#pragma once");
        sources.header.add("");
        sources.header.add("#include <string>");
        sources.header.add("#include <optional>");
        sources.header.add("#include <vector>");
        sources.header.add("#include \"pugixml.hpp\"");
        sources.header.add("");
        sources.header.addf("#include \"gaf_{}.h\"", name);
        sources.header.add("#include \"gaf/lib_pugixml.h\"");

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

std::string PugiXmlPlugin::get_name()
{
    return "pugixml";
}

int PugiXmlPlugin::run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args,
                              const std::string& name)
{
    if (auto r = no_arguments(args); r != 0)
    {
        return r;
    }

    auto out = xml::generate_xml(file, name);
    write_cpp(&out, writer, output_folder, name, "gaf_pugixml_");

    return 0;
}
