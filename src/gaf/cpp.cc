#include "gaf/cpp.h"

#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <filesystem>

#include "fmt/format.h"

#include "gaf/types.h"
#include "gaf/array.h"
#include "gaf/args.h"

// from gaf_types import StandardType, Struct, get_unique_types, File, Member, Enum, TypeList, Plugin

struct ImguiOptions
{
    std::string imgui_add = "\"+\"";
    std::string imgui_remove = "\"-\"";
};

struct Out
{
    std::vector<std::string> header;
    std::vector<std::string> source;

    void add_source(const std::string& line)
    {
        source.emplace_back(line);
    }

    void add_header(const std::string& line)
    {
        header.emplace_back(line);
    }
};


struct VarValue
{
    std::string variable;
    std::string value;
};


// todo(Gustav): remove this
std::string json_return_value()
{
    return "std::string";
}


// todo(Gustav): remove this
std::string json_is_false()
{
    return ".empty() == false";
}


std::string json_return_error(const std::string& err, const std::string& val)
{
    std::ostringstream ss;
    ss
        << "{ gaf_ss.str(\"\"); gaf_ss << \""
        << err
        << ", path: \" << gaf_path << \", value: \" << GafToString("
        << val
        << "); return gaf_ss.str(); }";
    return ss.str();
}


std::string json_return_ok()
{
    return "\"\"";
}


VarValue get_cpp_parse_from_rapidjson_helper_int(Out* sources, StandardType t, const std::string& member, const std::string& indent, const std::string& name, const std::string& json)
{
    const auto rti=json_return_error(fmt::format("read value for {} was not a integer", name), json);
    const auto rtl=json_return_error(fmt::format("read value for {} was to low", name), "gafv");
    const auto rth=json_return_error(fmt::format("read value for {} was to high", name), "gafv");
    std::ostringstream line;
    line << indent << "if(" << json << ".IsInt64()==false) " << rti << "\n";
    line << indent << "auto gafv = " << json << ".GetInt64();\n";
    line << indent << "if(gafv < std::numeric_limits<" << get_cpp_type(t) << ">::min()) " << rtl << "\n";
    line << indent << "if(gafv > std::numeric_limits<" << get_cpp_type(t) << ">::max()) " << rth << "\n";
    sources->add_source(line.str());
    const auto var = fmt::format("c->{}", member);
    const auto val = fmt::format("static_cast<{}>(gafv)", get_cpp_type(t));
    return VarValue{var, val};
}

VarValue get_cpp_parse_from_rapidjson_helper_float(Out* sources, const std::string& member, const std::string& indent, const std::string& name, const std::string& json)
{
    const auto err = json_return_error(fmt::format("read value for {} was not a number", name), json);
    std::ostringstream line;
    line << indent << "if(" << json << ".IsNumber()==false) " << err << " \n";
    sources->add_source(line.str());
    const auto var = fmt::format("c->{}", member);
    const auto val = fmt::format("{}.GetDouble()", json);
    return VarValue{var, val};
}

VarValue get_cpp_parse_from_rapidjson_base(Out* sources, StandardType t, const std::string& member, const std::string& indent, const std::string& name, const std::string& json)
{
    switch(t)
    {
    // todo: verify that all int parsing ranges are correct
    case StandardType::Int8:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json);
    case StandardType::Int16:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json);
    case StandardType::Int32:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json);
    case StandardType::Int64:
    {
        std::ostringstream line;
        const auto err = json_return_error(fmt::format("read value for {} was not a integer", name), json);
        line << indent << "if(" << json << ".IsInt64()==false) " << err << "} \n";
        const auto var = fmt::format("c->{}", member);
        const auto val = fmt::format("{}.GetInt64()", json);
        sources->add_source(line.str());
        return VarValue{var, val};
    }
    case StandardType::Uint8:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json);
    case StandardType::Uint16:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json);
    case StandardType::Uint32:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json);
    case StandardType::Uint64:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json);
    case StandardType::Float:
        return get_cpp_parse_from_rapidjson_helper_float(sources, member, indent, name, json);
    case StandardType::Double:
        return get_cpp_parse_from_rapidjson_helper_float(sources, member, indent, name, json);
    case StandardType::Byte:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json);
    case StandardType::Bool:
    {
        std::ostringstream line;
        const auto err = json_return_error(fmt::format("read value for {} was not a bool", name), json);
        line << indent << "if(" << json << ".IsBool()==false) " << err << "} \n";
        const auto var = fmt::format("c->{}", member);
        const auto val = fmt::format("{}.GetBool()", json);
        sources->add_source(line.str());
        return VarValue{var, val};
    }
    case StandardType::String:
    {
        std::ostringstream line;
        const auto err = json_return_error(fmt::format("read value for {} was not a string", name), json);
        line << indent << "if(" << json << ".IsString()==false) " << err << "} \n";
        const auto var = fmt::format("c->{}", member);
        const auto val = fmt::format("{}.GetString()", json);
        sources->add_source(line.str());
        return VarValue{var, val};
    }
    default:
        assert(false && "BUG: No type specified");
        return VarValue{member, "bug_unhandled_std_type"};
    }
}


void get_cpp_parse_from_rapidjson(Out* sources, const StandardType& t, const std::string& member,
                                  const std::string& indent, const std::string& name, const Member& member_type)
{
    if(member_type.is_dynamic_array)
    {
        std::ostringstream line;
        const auto err = json_return_error(fmt::format("tried to read {} but value was not a array", name), "arr");
        line << indent << "const rapidjson::Value& arr = iter->value;\n";
        line << indent << "if(!arr.IsArray()) " << err << "\n";
        line << indent << "for (rapidjson::SizeType i=0; i<arr.Size(); i++)\n";
        line << indent << "{\n";
        sources->add_source(line.str());
        const auto vv = get_cpp_parse_from_rapidjson_base(sources, t, member, indent + "  ", name, "arr[i]");
        sources->add_source(fmt::format("{}  {}.push_back({});\n", indent, vv.variable, vv.value));
        sources->add_source(indent + "}\n");
    }
    else
    {
        const auto vv = get_cpp_parse_from_rapidjson_base(sources, t, member, indent, name, "iter->value");
        if(member_type.is_optional)
        {
            sources->add_source(fmt::format("{}{} = std::make_shared<{}>({});\n", indent, vv.variable, get_cpp_type(t), vv.value));
        }
        else
        {
            sources->add_source(fmt::format("{}{} = {};\n", indent, vv.variable, vv.value));
        }
    }
}


void write_json_member(const Member& m, Out* sources, const std::string& indent)
{
    if(m.type_name.standard_type != StandardType::INVALID)
    {
        get_cpp_parse_from_rapidjson(sources, m.type_name.standard_type, m.name, indent, m.name, m);;
    }
    else
    {
        if(m.is_dynamic_array)
        {
            const auto lines = make_array<std::string>
            (
                "{i}const rapidjson::Value& arr = iter->value;\n",
                "{i}if(!arr.IsArray()) {err}\n",
                "{i}for (rapidjson::SizeType i=0; i<arr.Size(); i++)\n",
                "{i}{{\n",
                "{i}  {type} temp;\n",
                "{i}  gaf_ss.str(\"\");\n",
                "{i}  gaf_ss << gaf_path << \".{name}[\" << i << \"]\";\n",
                "{i}  {rv} r = ReadFromJsonValue(&temp,arr[i], gaf_ss.str());\n",
                "{i}  if(r{false}) {{ return r; }}\n",
                "{i}  c->{name}.push_back(temp);\n",
                "{i}}}\n"
            );
            for(const auto& line: lines)
            {
                const auto err = json_return_error(fmt::format("tried to read {} but value was not a array", m.name), "arr");
                sources->add_source
                (
                    fmt::format
                    (
                        line,
                        fmt::arg("i", indent),
                        fmt::arg("name", m.name),
                        fmt::arg("type", m.type_name.name),
                        fmt::arg("err", err),
                        fmt::arg("false", json_is_false()),
                        fmt::arg("rv", json_return_value())
                    )
                );
            }
        }
        else if(m.is_optional)
        {
            const auto lines = make_array<std::string>
            (
                "{i}c->{name} = std::make_shared<{type}>();\n"
                "{i}gaf_ss.str(\"\");\n"
                "{i}gaf_ss << gaf_path << \".{name}\";\n"
                "{i}{rv} r = ReadFromJsonValue(c->{name}.get(),iter->value, gaf_ss.str());\n"
                "{i}if(r{false})\n"
                "{i}{{\n"
                "{i}  c->{name}.reset();\n"
                "{i}  return r;\n"
                "{i}}}\n"
            );
            for(const auto& line: lines)
            {
                sources->add_source
                (
                    fmt::format
                    (
                        line,
                        fmt::arg("i", indent),
                        fmt::arg("name", m.name),
                        fmt::arg("type", m.type_name.name),
                        fmt::arg("false", json_is_false()),
                        fmt::arg("rv", json_return_value())
                    )
                );
            }
        }
        else
        {
            const auto lines = make_array<std::string>
            (
                "{i}gaf_ss.str("");\n"
                "{i}gaf_ss << gaf_path << \".{name}\";\n"
                "{i}{rv} r = ReadFromJsonValue(&c->{name},iter->value, gaf_ss.str());\n"
                "{i}if(r{false})\n"
                "{i}{{\n"
                "{i}  return r;\n"
                "{i}}}\n"
            );
            for(const auto& line: lines)
            {
                sources->add_source
                (
                    fmt::format
                    (
                        line,
                        fmt::arg("i", indent),
                        fmt::arg("name", m.name),
                        fmt::arg("false", json_is_false()),
                        fmt::arg("rv", json_return_value())
                    )
                );
            }
        }
    }
}


void write_json_source_for_cpp(Out* sources, const Struct& s)
{
    sources->add_source(fmt::format("{} ReadFromJsonValue({}* c, const rapidjson::Value& value, const std::string& gaf_path) {{\n", json_return_value(), s.name));
    sources->add_source("  std::stringstream gaf_ss;\n");
    sources->add_source(fmt::format("  if(!value.IsObject()) {}\n", json_return_error(fmt::format("tried to read {} but value was not a object", s.name), "value")));
    sources->add_source("  rapidjson::Value::ConstMemberIterator iter;\n");
    for(const auto& m: s.members)
    {
        sources->add_source(fmt::format("  iter = value.FindMember(\"{}\");\n", m.name));
        sources->add_source("  if(iter != value.MemberEnd()) {\n");
        write_json_member(m, sources, "    ");
        sources->add_source("  }\n");
        if(m.missing_is_fail || m.is_optional)
        {
            sources->add_source("  else {\n");
            if(m.is_optional)
            {
                sources->add_source(fmt::format("    c->{}.reset();\n", m.name));
            }
            else
            {
                sources->add_source(fmt::format("    {}\n", json_return_error(fmt::format("missing {} in json object", m.name), "value")));
            }
            sources->add_source("  }\n");
        }
    }
    sources->add_source(fmt::format("  return {};\n", json_return_ok()));
    sources->add_source("}\n");
    sources->add_source("\n");
}


std::string determine_pushback_value(const Member& m)
{
    const auto t = m.type_name;
    if(t.standard_type == StandardType::String)
    {
        return "\"\"";
    }
    auto tl = TypeList{};
    tl.add_default_types();
    if(tl.is_valid_type(t.name))
    {
        const auto nt = tl.get_type(t.name);
        return *nt.default_value;
    }
    else
    {
        return fmt::format("{}()", t.name);
    }
}


void add_imgui_delete_button(const Member&, Out* sources, const ImguiOptions& opt)
{
    sources->add_source(fmt::format("      if( ImGui::Button({}) )\n", opt.imgui_remove));
    sources->add_source("      {\n");
    sources->add_source("        delete_index = i;\n");
    sources->add_source("        please_delete = true;\n");
    sources->add_source("      }\n");
}


void write_single_imgui_member_to_source(const std::string& name, const std::string& var, const StandardType& t, Out* sources, const std::string& indent, const Member& m, bool add_delete, const ImguiOptions& opt)
{
    switch(t)
    {
    case StandardType::Int8:
        sources->add_source(fmt::format("{}ImGui::Edit({}, {});\n", indent, name, var));
        return;
    case StandardType::Int16:
        sources->add_source(fmt::format("{}ImGui::Edit({}, {});\n", indent, name, var));
        return;
    case StandardType::Int32:
        sources->add_source(fmt::format("{}ImGui::InputInt({}, {});\n", indent, name, var));
        return;
    case StandardType::Int64:
        sources->add_source(fmt::format("{}ImGui::Edit({}, {});\n", indent, name, var));
        return;
    case StandardType::Uint8:
        sources->add_source(fmt::format("{}ImGui::Edit({}, {});\n", indent, name, var));
        return;
    case StandardType::Uint16:
        sources->add_source(fmt::format("{}ImGui::Edit({}, {});\n", indent, name, var));
        return;
    case StandardType::Uint32:
        sources->add_source(fmt::format("{}ImGui::Edit({}, {});\n", indent, name, var));
        return;
    case StandardType::Uint64:
        sources->add_source(fmt::format("{}ImGui::Edit({}, {});\n", indent, name, var));
        return;
    case StandardType::Float:
        sources->add_source(fmt::format("{}ImGui::InputFloat({}, {});\n", indent, name, var));
        return;
    case StandardType::Double:
        sources->add_source(fmt::format("{}ImGui::InputDouble({}, {});\n", indent, name, var));
        return;
    case StandardType::Byte:
        sources->add_source(fmt::format("{}ImGui::Edit({}, {});\n", indent, name, var));
        return;
    case StandardType::Bool:
        sources->add_source(fmt::format("{}ImGui::Checkbox({}, {});\n", indent, name, var));
        return;
    case StandardType::String:
        sources->add_source(fmt::format("{}{{\n", indent));
        sources->add_source(fmt::format("{}  char gaf_temp[1024];\n", indent));
        sources->add_source(fmt::format("{}  strcpy(gaf_temp, ({})->c_str());\n", indent, var));
        sources->add_source(fmt::format("{}  if(ImGui::InputText({}, gaf_temp, 1024))\n", indent, name));
        sources->add_source(fmt::format("{}  {{\n", indent));
        sources->add_source(fmt::format("{}    *({}) = gaf_temp;\n", indent, var));
        sources->add_source(fmt::format("{}  }}\n", indent));
        sources->add_source(fmt::format("{}}}\n", indent));
        return;
    default:
        if(m.type_name.is_enum)
        {
            sources->add_source(fmt::format("{}RunImgui({}, {});\n", indent, var, name));
        }
        else
        {
            sources->add_source
            (
                fmt::format
                (
                    "{}if(ImGui::TreeNodeEx({}, ImGuiTreeNodeFlags_DefaultOpen{}))\n",
                    indent,
                    name,
                    add_delete ? "| ImGuiTreeNodeFlags_FramePadding" : ""
                )
            );
            sources->add_source(fmt::format("{}{{\n", indent));
            if(add_delete)
            {
                sources->add_source(fmt::format("{}  ImGui::SameLine();\n", indent));
                add_imgui_delete_button(m, sources, opt);
            }
            sources->add_source(fmt::format("{}  RunImgui({});\n", indent, var));
            sources->add_source(fmt::format("{}  ImGui::TreePop();\n", indent));
            sources->add_source(fmt::format("{}}}\n", indent));
            if(add_delete)
            {
                sources->add_source(fmt::format("{}else\n", indent));
                sources->add_source(fmt::format("{}{{\n", indent));
                sources->add_source(fmt::format("{}  ImGui::SameLine();\n", indent));
                add_imgui_delete_button(m, sources, opt);
                sources->add_source(fmt::format("{}}}\n", indent));
            }
        }
        return;
    }
}


std::string determine_new_value(const Member& m)
{
    const auto t = m.type_name;
    auto tl = TypeList{};
    tl.add_default_types();
    if(tl.is_valid_type(t.name) && t.standard_type != StandardType::String)
    {
        const auto nt = tl.get_type(t.name);
        return fmt::format("new {}({})", t.get_cpp_type(), *nt.default_value);
    }
    else
    {
        return fmt::format("new {}()", t.name);
    }
}


void write_single_member_to_source(const Member& m, Out* sources, const ImguiOptions& opt)
{
    if(m.is_dynamic_array == false)
    {
        if(m.is_optional)
        {
            sources->add_source(fmt::format("    if(c->{})\n", m.name));
            sources->add_source(fmt::format("    {\n"));
            write_single_imgui_member_to_source
            (
                fmt::format("\"{}\"", m.name),
                fmt::format("c->{}.get()", m.name),
                m.type_name.standard_type,
                sources, "      ", m, false, opt
            );
            sources->add_source(fmt::format("      if(ImGui::Button(\"Clear {}\")) {{ c->{name}.reset(); }}\n", m.name));
            sources->add_source("    }\n");
            sources->add_source("    else\n");
            sources->add_source("    {\n");
            sources->add_source(fmt::format("      if(ImGui::Button(\"Set {0}\")) {{ c->{0}.reset({1}); }}\n", m.name, determine_new_value(m)));
            sources->add_source("    }\n");
            sources->add_source("    \n");
        }
        else
        {
            write_single_imgui_member_to_source
            (
                fmt::format("\"{}\"", m.name),
                fmt::format("&c->{}", m.name),
                m.type_name.standard_type,
                sources, "  ", m, false, opt
            );
        }
    }
    else
    {
        const auto short_version = m.type_name.standard_type != StandardType::INVALID || m.type_name.is_enum;
        sources->add_source(fmt::format("  if(ImGui::TreeNodeEx(\"{}\", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))\n", m.name));
        sources->add_source("  {\n");
        sources->add_source("    ImGui::SameLine();\n");
        sources->add_source(fmt::format("    if(ImGui::Button({}))\n", opt.imgui_add));
        sources->add_source("    {\n");
        sources->add_source(fmt::format("      c->{}.push_back({});\n", m.name, determine_pushback_value(m)));
        sources->add_source("    }\n");
        sources->add_source("    std::size_t delete_index = 0;\n");
        sources->add_source("    bool please_delete = false;\n");
        sources->add_source(fmt::format("    for(std::size_t i=0; i<c->{}.size(); i+= 1)\n", m.name));
        sources->add_source("    {\n");
        sources->add_source("      std::stringstream gaf_ss;\n");
        sources->add_source(fmt::format("      gaf_ss << \"{}[\" << i << \"]\";\n", m.name));
        sources->add_source("      ImGui::PushID(i);\n");
        write_single_imgui_member_to_source("gaf_ss.str().c_str()", fmt::format("&c->{}[i]", m.name), m.type_name.standard_type, sources, "      ", m, !short_version, opt);
        if(short_version)
        {
            sources->add_source(fmt::format("      ImGui::SameLine();\n"));
            add_imgui_delete_button(m, sources, opt);
        }
        sources->add_source("      ImGui::PopID();\n");
        sources->add_source("    }\n");
        sources->add_source("    if(please_delete)\n");
        sources->add_source("    {\n");
        sources->add_source(fmt::format("      c->{0}.erase(c->{0}.begin()+delete_index);\n", m.name));
        sources->add_source("    }\n");
        sources->add_source("    ImGui::TreePop();\n");
        sources->add_source("  }\n");
        sources->add_source("  else\n");
        sources->add_source("  {\n");
        sources->add_source("    ImGui::SameLine();\n");
        sources->add_source(fmt::format("    if(ImGui::Button({}))\n", opt.imgui_add));
        sources->add_source("    {\n");
        sources->add_source(fmt::format("      c->{}.push_back({});\n", m.name, determine_pushback_value(m)));
        sources->add_source("    }\n");
        sources->add_source("  }\n");
    }
}


void write_imgui_source_for_cpp(Out* sources, const Struct& s, const ImguiOptions& opt)
{
    sources->add_source(fmt::format("void RunImgui({}* c)\n", s.name));
    sources->add_source("{\n");
    for(const auto& m: s.members)
    {
        write_single_member_to_source(m, sources, opt);
    }
    sources->add_source("}\n");
    sources->add_source("\n");
}


void write_member_variables_for_cpp(Out* sources, const Struct& s)
{
    for(const auto& m: s.members)
    {
        // m.type_name.is_enum
        const auto type_name = m.type_name.name;
        if(m.is_optional)
        {
            sources->add_header(fmt::format("  std::shared_ptr<{}> {};\n", type_name, m.name));
        }
        else if(m.is_dynamic_array)
        {
            sources->add_header(fmt::format("  std::vector<{}> {};\n", type_name, m.name));
        }
        else
        {
            sources->add_header(fmt::format("  {} {};\n", type_name, m.name));
        }
    }
    sources->add_header(fmt::format("}}; // class {}\n", s.name));
}

template<typename T, typename Predicate>
std::vector<T> filter(const std::vector<T>& src, const Predicate& p)
{
    std::vector<T> r;
    std::copy_if(src.begin(), src.end(), std::back_inserter(r), p);
    return r;
}

void write_default_constructor_for_cpp(const Struct& s, Out* sources)
{
    const auto common_members = filter(s.members, [](const Member& x) {return x.defaultvalue.has_value();});
    if(common_members.empty()) { return; }

    sources->add_header(fmt::format("  {}();\n", s.name));
    sources->add_header("\n");
    sources->add_source(fmt::format("{0}::{0}()\n", s.name));
    auto sep = ':';
    for(const auto& m: common_members)
    {
        auto default_value = *m.defaultvalue;
        if(m.type_name.is_enum)
        {
            default_value = fmt::format("{}::{}", m.type_name.name, *m.defaultvalue);
        }
        sources->add_source(fmt::format("  {} {}({})\n", sep, m.name, default_value));
        sep = ',';
    }
    sources->add_source("{}\n");
    sources->add_source("\n");
}


void iterate_enum(const Enum& e, Out* sources, bool prefix_prop=false)
{
    const auto prefix = prefix_prop ? fmt::format("{}_", e.name) : "";
    std::size_t index = 0;
    for(const auto& v: e.values)
    {
        const auto last = index == e.values.size();
        const auto comma = last ? "" : ",";
        sources->add_header(fmt::format("  {}{}{}\n", prefix, v, comma));
        index += 1;
    }
}


std::string get_value_prefix_opt(const Enum& e)
{
    return fmt::format("{}::", e.name);
}


void add_enum_json_function(const Enum& e, Out* sources, bool type_enum=false)
{
    const auto enum_type = type_enum ? fmt::format("{}::Type", e.name) : e.name;
    const auto value_prefix = get_value_prefix_opt(e);
    const auto arg = ", const std::string& gaf_path";
    sources->add_header(fmt::format("{} ReadFromJsonValue({}* c, const rapidjson::Value& value{});\n", json_return_value(), enum_type, arg));
    sources->add_source(fmt::format("{} ReadFromJsonValue({}* c, const rapidjson::Value& value{})\n", json_return_value(), enum_type, arg));
    sources->add_source("{\n");
    sources->add_source("  std::stringstream gaf_ss;\n");
    sources->add_source(fmt::format("  if(value.IsString()==false) {};\n", json_return_error(fmt::format("read value for {} was not a string", e.name), "value")));
    for(const auto& v: e.values)
    {
        sources->add_source
        (
            fmt::format
            (
                "  if(strcmp(value.GetString(), \"{v}\")==0) {{ *c = {p}{v}; return {ok};}}\n",
                fmt::arg("v", v),
                fmt::arg("p", value_prefix),
                fmt::arg("ok", json_return_ok())
            )
        );
    }
    sources->add_source(fmt::format("  {}\n", json_return_error(fmt::format("read string for {} was not valid", e.name), "value")));
    sources->add_source("}\n");
    sources->add_source("\n");
}


Out generate_json(const File& f, const std::string& name)
{
    auto sources = Out{};
    
    sources.add_header("#pragma once\n");
    sources.add_header("\n");

    sources.add_header("#include <string>\n");
    sources.add_source("#include <cstring>\n");
    sources.add_header("#include \"rapidjson/document.h\"\n");
    sources.add_header("\n");
    sources.add_header(fmt::format("#include \"gaf_{}.h\"\n", name));
    sources.add_header("\n");

    if(f.package_name.empty() == false)
    {
        sources.add_header(fmt::format("namespace {} {{\n", f.package_name));
        sources.add_header("\n");
    }

    if(f.typedefs.empty() == false)
    {
        for(const auto& s: f.typedefs)
        {
            sources.add_header(fmt::format("class {};\n", s->name));
        }
        sources.add_header("\n");
    }

    for(const auto& e: f.enums)
    {
        add_enum_json_function(*e, &sources);
    }

    for(const auto& s: f.structs_defined)
    {
        write_json_source_for_cpp(&sources, *s);
        
        sources.add_header("\n");
        const auto arg = ", const std::string& gaf_path";
        sources.add_header(fmt::format("{} ReadFromJsonValue({}* c, const rapidjson::Value& value{});\n", json_return_value(), s->name, arg));
        sources.add_header("\n");
    }
    
    sources.add_header("std::string GafToString(const rapidjson::Value& val);\n");
    sources.add_source("std::string GafToString(const rapidjson::Value& val)\n");
    sources.add_source("{\n");
    sources.add_source("  if(val.IsNull()) { return \"Null\"; };\n");
    sources.add_source("  if(val.IsFalse()) { return \"False\"; };\n");
    sources.add_source("  if(val.IsTrue()) { return \"True\"; };\n");
    sources.add_source("  if(val.IsObject()) { return \"Object\"; };\n");
    sources.add_source("  if(val.IsArray()) { return \"Array\"; };\n");
    sources.add_source("  if(val.IsUint64()) { std::stringstream ss; ss << \"uint of \" << val.GetUint64(); return ss.str(); };\n");
    sources.add_source("  if(val.IsInt64()) { std::stringstream ss; ss << \"int of \" << val.GetInt64(); return ss.str(); };\n");
    sources.add_source("  if(val.IsDouble()) { std::stringstream ss; ss << \"double of \" << val.GetDouble(); return ss.str(); };\n");
    sources.add_source("  if(val.IsString()) { std::stringstream ss; ss << \"string of \" << val.GetString(); return ss.str(); };\n");
    sources.add_source("  return \"<unknown>\";\n");
    sources.add_source("}\n");
    sources.add_source("\n");

    // todo: remove this horrible function
    sources.add_header("std::string GafToString(int64_t val);\n");
    sources.add_source("std::string GafToString(int64_t val)\n");
    sources.add_source("{\n");
    sources.add_source("  std::stringstream ss;\n");
    sources.add_source("  ss << val;\n");
    sources.add_source("  return ss.str();\n");
    sources.add_source("}\n");
    sources.add_source("\n");

    sources.add_header("\n");

    if(f.package_name.empty() == false)
    {
        sources.add_header(fmt::format("}} // namespace {}\n", f.package_name));
        sources.add_header("\n");
    }
    sources.add_header("\n");

    return sources;
}


Out generate_imgui(const File& f, const std::string& name, const ImguiOptions& opt)
{
    auto sources = Out{};

    sources.add_header("#pragma once\n");
    sources.add_header("\n");
    sources.add_header(fmt::format("#include \"gaf_{}.h\"\n", name));
    sources.add_header("\n");

    if(f.package_name.empty() == false)
    {
        sources.add_header(fmt::format("namespace {} {{\n", f.package_name));
        sources.add_header("\n");
    }

    if(f.typedefs.empty() == false)
    {
        for(const auto& s: f.typedefs)
        {
            sources.add_header(fmt::format("class {};\n", s->name));
        }
        sources.add_header("\n");
    }

    for(const auto& e: f.enums)
    {
        sources.add_header(fmt::format("const char* ToString({} en);\n", name));
        sources.add_source(fmt::format("const char* ToString({} en)\n", name));
        sources.add_source("{\n");
        for(const auto& v: e->values)
        {
            sources.add_source(fmt::format("  if(en == {0}{1}) {{ return \"{1}\"; }}\n", get_value_prefix_opt(*e), v));
        }
        sources.add_source("  return \"<invalid value>\";\n");
        sources.add_source("}\n");

        sources.add_header(fmt::format("void RunImgui({}* en, const char* label);\n", e->name));
        sources.add_source(fmt::format("void RunImgui({}* en, const char* label)\n", e->name));
        sources.add_source("{\n");
        sources.add_source("  if(ImGui::BeginCombo(label, ToString(*en)))\n");
        sources.add_source("  {\n");
        for(const auto& v: e->values)
        {
            sources.add_source(fmt::format("    if(ImGui::Selectable(\"{0}\", *en == {1}{0})) {{ *en = {prefix}{val}; }}\n", v, get_value_prefix_opt(*e)));
        }
        sources.add_source("    ImGui::EndCombo();\n");
        sources.add_source("  }\n");
        sources.add_source("}\n");
        sources.add_header("\n");
        sources.add_source("\n");
    }
    
    for(const auto& s: f.structs_defined)
    {
        write_imgui_source_for_cpp(&sources, *s, opt);

        sources.add_header("\n");
        sources.add_header(fmt::format("void RunImgui({}* c);\n", s->name));
    }
    
    if(f.package_name.empty() == false)
    {
        sources.add_header(fmt::format("}} // namespace {}\n", f.package_name));
        sources.add_header("\n");
    }
    sources.add_header("\n");

    return sources;
}

template<typename T>
bool is_in(const T& t, const std::vector<T>& tt)
{
    return std::find(tt.begin(), tt.end(), t) != tt.end();
}

template<typename T, typename F, typename C>
std::vector<T> map(const std::vector<F>& f, const C& c)
{
    std::vector<T> r;
    transform(f.begin(), f.end(), back_inserter(r), c);
    return r;
}

template<typename T, typename C>
bool any(const T& t, const C& c)
{
    return std::any_of(t.begin(), t.end(), c);
}

Out generate_cpp(const File& f)
{
    auto sources = Out{};

    // get all standard types used for typedefing later on...
    const auto unique_types = get_unique_types(f);
    const auto default_types = filter
    (
        unique_types,
        [](const Type& t)
        {
            return get_cpp_type(t.standard_type) != "" && t.name != t.get_cpp_type();
        }
    );

    const auto has_string = is_in(StandardType::String, map<StandardType>(unique_types, [](const Type& t) {return t.standard_type;}));

    // has_dynamic_arrays = any(m for s in f.structs for m in s.members if m.is_dynamic_array);
    // has_optional = any(m for s in f.structs for m in s.members if m.is_optional);
    const auto has_dynamic_arrays = any(map<bool>(f.structs, [](const auto& s){ return filter(s->members, [](const auto& m) { return m.is_dynamic_array; }).empty(); }), [](bool b){return b;});
    const auto has_optional = any(map<bool>(f.structs, [](const auto& s){ return filter(s->members, [](const auto& m) { return m.is_optional; }).empty(); }), [](bool b){return b;});

    sources.add_header("#pragma once\n");
    sources.add_header("\n");

    bool added_include = false;
    if(default_types.empty() == false)
    {
        added_include = true;
        sources.add_header("#include <cstdint>\n");
    }
    if(has_string)
    {
        added_include = true;
        sources.add_header("#include <string>\n");
    }
        
    if(has_dynamic_arrays)
    {
        added_include = true;
        sources.add_header("#include <vector>\n");
    }
    if(has_optional)
    {
        added_include = true;
        sources.add_header("#include <memory>\n");
    }
    if(added_include)
    {
        sources.add_header("\n");
    }

    if(f.package_name.empty() == false)
    {
        sources.add_header(fmt::format("namespace {} {{\n", f.package_name));
        sources.add_header("\n");
    }

    if(default_types.empty() == false)
    {
        for(const auto& t: default_types)
        {
            sources.add_header(fmt::format("typedef {} {};\n", t.get_cpp_type(), t.name));
        }
        sources.add_header("\n");
    }

    if(f.typedefs.empty() == false)
    {
        for(const auto& s: f.typedefs)
        {
            sources.add_header(fmt::format("class {};\n", s->name));
        }
        sources.add_header("\n");
    }

    for(const auto& e: f.enums)
    {
        sources.add_header(fmt::format("enum class {} {{\n", e->name));
        iterate_enum(*e, &sources);
        sources.add_header(fmt::format("}}; // enum {}\n", e->name));

        sources.add_header("\n");
    }

    for(const auto& s: f.structs_defined)
    {
        sources.add_header(fmt::format("class {} {{\n", s->name));
        sources.add_header(" public:\n");
        write_default_constructor_for_cpp(*s, &sources);

        write_member_variables_for_cpp(&sources, *s);
        sources.add_header("\n");
    }

    if(f.package_name.empty() == false)
    {
        sources.add_header(fmt::format("}} // namespace {}\n", f.package_name));
        sources.add_header("\n");
    }
    sources.add_header("\n");

    return sources;
}

std::string get_file_path(const std::string& folder, const std::string& name)
{
    const auto r = std::filesystem::path{folder} / std::filesystem::path{name};
    const auto a = std::filesystem::absolute(r);
    return a.native();
}

void write_cpp(Out* sources, Writer* writer, const std::string& out_dir, const std::string& name, const std::string& prefix, const std::string& package_name, const std::vector<std::string>& includes)
{
    if(auto out = writer->open(get_file_path(out_dir, prefix + name + ".h")); out != nullptr)
    {
        for(const auto& s: sources->header)
        {
            out->write(s);
        }
    }

    if(auto out = writer->open(get_file_path(out_dir, prefix + name + ".cc")); out != nullptr)
    {
        out->write(fmt::format("#include \"{}.h\"\n", prefix + name));
        out->write("\n");
        for(const auto& inc: includes)
        {
            out->write(fmt::format("#include {}\n", inc));
        }
        out->write("\n");

        if(package_name.empty() == false)
        {
            out->write(fmt::format("namespace {} {{\n", package_name));
            out->write("\n");
        }

        for(const auto& s: sources->source)
        {
            out->write(s);
        }

        if(package_name.empty() == false)
        {
            out->write(fmt::format("}} // namespace {}\n", package_name));
            out->write("\n");
        }
    }
}


std::string CppPlugin::get_name()
{
    return "cpp";
}

int CppPlugin::run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args, const std::string& name)
{
    if(auto r = no_arguments(args); r != 0)
    {
        return r;
    }

    auto out = generate_cpp(file);
    write_cpp(&out, writer, output_folder, name, "gaf_", file.package_name, {"<limits>"});

    return 0;
}

std::string RapidJsonPlugin::get_name()
{
    return "rapidjson";
}

int RapidJsonPlugin::run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args, const std::string& name)
{
    if(auto r = no_arguments(args); r != 0)
    {
        return r;
    }

    auto out = generate_json(file, name);
    write_cpp(&out, writer, output_folder, name, "gaf_rapidjson_", file.package_name, {"<sstream>", "\"rapidjson/document.h\""});

    return 0;
}

std::string ImguiPlugin::get_name()
{
    return "imgui";
}

int ImguiPlugin::run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args, const std::string& name)
{
    std::vector<std::string> imgui_headers = {"\"imgui.h\""};
    std::string imgui_add = "\"Add\"";
    std::string imgui_remove = "\"Remove\"";

    while(args.has_more())
    {
        const auto c = args.read();
        if(c == "--imgui-add")
        {
            imgui_add = args.read();
        }
        else if(c == "--imgui-remove")
        {
            imgui_remove = args.read();
        }
        else if(c == "--imgui-headers")
        {
            imgui_headers.clear();
            while(is_option(args.peek()) == false)
            {
                imgui_headers.emplace_back(args.read());
            }
        }
        else
        {
            std::cerr << "invalid option " << c << "\n";
            return -42;
        }
    }
    
    
    auto out = generate_imgui(file, name, ImguiOptions{imgui_add, imgui_remove});
    write_cpp(&out, writer, output_folder, name, "gaf_imgui_", file.package_name, imgui_headers);
    
    return 0;
}
