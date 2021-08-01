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

        const auto signature =
            fmt::format("std::string ParseEnumString({}* c, const char* value)", e.name);
        sources->header.addf("{};", signature);
        sources->source.add(signature);
        sources->source.add("{");
        for (const auto& v : e.values)
        {
            sources->source.addf(
                "if(strcmp(value, \"{value}\") == 0) {{ *c = {type}::{value}; return \"\"; }}",
                fmt::arg("type", e.name), fmt::arg("value", v));
        }
        // todo(Gustav): add list of valid values with a could_be_fun
        sources->source.addf("return fmt::format(\"{{}} is not a valid name for enum {}\", value);",
                             e.name);
        sources->source.add("}");
        sources->source.add("");
    }

    void add_member_failure_to_read(Out* sources, const Member& m)
    {
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
                sources->source.addf("return \"{} is missing\";", m.name);
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
        case StandardType::Byte:
        case StandardType::Bool:
        case StandardType::String: return true;
        default: return false;
        }
    }

    void add_member_variable_array(Out* sources, const Member& m)
    {
        if (m.type_name.is_enum)
        {
            sources->source.addf("for(const auto el: value.children(\"{}\"))", m.name);
            sources->source.add("{");
            sources->addf("auto em = {}{{}};", m.type_name.get_cpp_type());
            sources->source.add(
                "if(const auto error = ParseEnumString(&em, el.child_value()); error.empty() == "
                "false)");
            sources->source.add("{");
            sources->source.add("return error;");
            sources->source.add("}");
            sources->source.addf("c->{}.emplace_back(em)", m.name);
            sources->source.add("}");
        }
        else if (is_basic_type(m.type_name))
        {
            sources->source.addf("for(const auto el: value.children(\"{}\"))", m.name);
            sources->source.add("{");
            switch (m.type_name.standard_type)
            {
            case StandardType::Bool:
                sources->source.add("bool b = false;");
                sources->source.add("if(gaf::parse_bool(&b, el.child_value()) == false)");
                sources->source.add("{");
                sources->source.addf(
                    "return fmt::format(\"Invalid bool for {}: {{}}\", el.child_value());", m.name);
                sources->source.add("}");
                sources->source.addf("c->{}.emplace_back(b);", m.name);
                break;
            case StandardType::String:
                sources->source.addf("c->{}.emplace_back(el.child_value());", m.name);
                break;
            default:
                sources->source.addf("{} v = 0;", m.type_name.get_cpp_type());
                sources->source.add("std::istringstream ss(el.child_value());");
                sources->source.add("ss >> v;");
                sources->source.add("if(ss.good() == false)");
                sources->source.add("{");
                sources->source.addf(
                    "return fmt::format(\"Invalid format for {}: {{}}\", el.child_value());", m.name);
                sources->source.add("}");
                sources->source.addf("c->{}.emplace_back(v);", m.name);
                break;
            }
            sources->source.add("}");
        }
        else
        {
            sources->source.addf("for(const auto child: value.children(\"{}\"))", m.name);
            sources->source.add("{");
            sources->source.addf("auto v = {}{{}};", m.type_name.get_cpp_type());
            sources->source.addf(
                "if(const auto error = ReadXmlElement(&v, value); error.empty() == false)", m.name);
            sources->source.add("{");
            sources->source.add("return error;");
            sources->source.add("}");
            sources->source.addf("c->{}.emplace_back(v);", m.name);
            sources->source.add("}");
        }
    }

    void add_member_variable_single(Out* sources, const Member& m)
    {
        if (m.type_name.is_enum)
        {
            sources->source.addf("if(const auto el = value.attribute(\"{}\"); el)", m.name);
            sources->source.add("{");
            sources->source.addf(
                "if(const auto error = ParseEnumString(&c->{}, el.value()); error.empty() == false)",
                m.name);
            sources->source.add("{");
            sources->source.add("return error;");
            sources->source.add("}");
            sources->source.add("}");
            add_member_failure_to_read(sources, m);
        }
        else if (is_basic_type(m.type_name))
        {
            sources->source.addf("if(const auto el = value.attribute(\"{}\"); el)", m.name);
            sources->source.add("{");
            switch (m.type_name.standard_type)
            {
            case StandardType::Bool:
                sources->source.addf("if(gaf::parse_bool(&c->{}, el.value()) == false)", m.name);
                sources->source.add("{");
                sources->source.addf("return fmt::format(\"Invalid bool for {}: {{}}\", el.value());",
                                     m.name);
                sources->source.add("}");
                break;
            case StandardType::String: sources->source.addf("c->{} = el.value();", m.name); break;
            default:
                sources->source.add("std::istringstream ss(el.value());");
                sources->source.addf("ss >> c->{};", m.name);
                sources->source.add("if(ss.good() == false)");
                sources->source.add("{");
                sources->source.addf("return fmt::format(\"Invalid format for {}: {{}}\", el.value());",
                                     m.name);
                sources->source.add("}");
                break;
            }
            sources->source.add("}");
            add_member_failure_to_read(sources, m);
        }
        else
        {
            sources->source.addf("if(const auto child = value.child(\"{}\"); child)", m.name);
            sources->source.add("{");
            sources->source.addf(
                "if(const auto error = ReadXmlElement(&c->{}, value); error.empty() == false)", m.name);
            sources->source.add("{");
            sources->source.add("return error;");
            sources->source.add("}");
            sources->source.add("}");
            add_member_failure_to_read(sources, m);
        }
    }

    void add_member_variable(Out* sources, const Member& m)
    {
        if (m.is_dynamic_array)
        {
            add_member_variable_array(sources, m);
        }
        else
        {
            add_member_variable_single(sources, m);
        }
    }

    void add_struct_function(Out* sources, const Struct& s)
    {
        const auto signature =
            fmt::format("std::string ReadXmlElement({}* c, const pugi::xml_node& value)", s.name);
        sources->header.addf("{};", signature);
        sources->source.add(signature);
        sources->source.add("{");
        for (const auto& m : s.members) { add_member_variable(sources, m); }
        sources->source.add("return \"\";");
        sources->source.add("}");
        sources->source.add("");
    }

    Out generate_xml(const File& f, const std::string& name)
    {
        auto sources = Out{};

        sources.header.add("#pragma once");
        sources.header.add("");
        sources.header.add("#include <string>");
        sources.header.add("#include \"pugixml.hpp\"");
        sources.header.add("");
        sources.header.addf("#include \"gaf_{}.h\"", name);

        sources.source.add("#include <cstring>");
        sources.source.add("#include <sstream>");
        sources.source.add("#include \"fmt/format.h\"");

        sources.add("");

        if (f.package_name.empty() == false)
        {
            sources.addf("namespace {}", f.package_name);
            sources.add("{");
        }

        for (const auto& e : f.enums) { add_enum_function(*e, &sources); }

        for (const auto& s : f.structs) { add_struct_function(&sources, *s); }

        if (f.package_name.empty() == false)
        {
            sources.add("}");
            sources.add("");
        }

        return sources;
    }
}

std::string PugiXmlPlugin::get_name() { return "pugixml"; }

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
