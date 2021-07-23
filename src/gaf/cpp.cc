#include "gaf/cpp.h"

#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <cassert>

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

struct Lines
{
    std::vector<std::string> lines;

    void add(const std::string& str)
    {
        if(str.find('\n') == std::string::npos)
        {
        }
        else
        {
            int i = 0;
            i+= 1;
        }
        
        assert(str.find('\n') == std::string::npos);
        lines.emplace_back(str);
    }
};

struct Out
{
    Lines header;
    Lines source;
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
    sources->source.add(fmt::format("if({}.IsInt64()==false) {}", json, rti));
    sources->source.add(fmt::format("auto gafv = {}.GetInt64();", json));
    sources->source.add(fmt::format("if(gafv < std::numeric_limits<{}>::min()) {}", get_cpp_type(t), rtl));
    sources->source.add(fmt::format("if(gafv > std::numeric_limits<{}>::max()) {}", get_cpp_type(t), rth));
    const auto var = fmt::format("c->{}", member);
    const auto val = fmt::format("static_cast<{}>(gafv)", get_cpp_type(t));
    return VarValue{var, val};
}

VarValue get_cpp_parse_from_rapidjson_helper_float(Out* sources, const std::string& member, const std::string& indent, const std::string& name, const std::string& json)
{
    const auto err = json_return_error(fmt::format("read value for {} was not a number", name), json);
    sources->source.add(fmt::format("if({}.IsNumber()==false) {}", json, err));
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
        const auto err = json_return_error(fmt::format("read value for {} was not a integer", name), json);
        sources->source.add(fmt::format("if({}.IsInt64()==false) {{ {} }}", json, err));
        const auto var = fmt::format("c->{}", member);
        const auto val = fmt::format("{}.GetInt64()", json);
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
        const auto err = json_return_error(fmt::format("read value for {} was not a bool", name), json);
        sources->source.add(fmt::format("if({}.IsBool()==false) {{ {} }}", json, err));
        const auto var = fmt::format("c->{}", member);
        const auto val = fmt::format("{}.GetBool()", json);
        return VarValue{var, val};
    }
    case StandardType::String:
    {
        const auto err = json_return_error(fmt::format("read value for {} was not a string", name), json);
        sources->source.add(fmt::format("if({}.IsString()==false) {{ {} }}", json, err));
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
                                  const std::string& indent, const std::string& name, const Member& member_type)
{
    if(member_type.is_dynamic_array)
    {
        const auto err = json_return_error(fmt::format("tried to read {} but value was not a array", name), "arr");
        sources->source.add("const rapidjson::Value& arr = iter->value;");
        sources->source.add(fmt::format("if(!arr.IsArray()) {}", err));
        sources->source.add("for (rapidjson::SizeType i=0; i<arr.Size(); i++)");
        sources->source.add("{");
        const auto vv = get_cpp_parse_from_rapidjson_base(sources, t, member, indent + "  ", name, "arr[i]");
        sources->source.add(fmt::format("{}  {}.push_back({});", indent, vv.variable, vv.value));
        sources->source.add(indent + "}");
    }
    else
    {
        const auto vv = get_cpp_parse_from_rapidjson_base(sources, t, member, indent, name, "iter->value");
        if(member_type.is_optional)
        {
            sources->source.add(fmt::format("{}{} = std::make_shared<{}>({});", indent, vv.variable, get_cpp_type(t), vv.value));
        }
        else
        {
            sources->source.add(fmt::format("{}{} = {};", indent, vv.variable, vv.value));
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
                "{i}const rapidjson::Value& arr = iter->value;",
                "{i}if(!arr.IsArray()) {err}",
                "{i}for (rapidjson::SizeType i=0; i<arr.Size(); i++)",
                "{i}{{",
                "{i}  {type} temp;",
                "{i}  gaf_ss.str(\"\");",
                "{i}  gaf_ss << gaf_path << \".{name}[\" << i << \"]\";",
                "{i}  {rv} r = ReadFromJsonValue(&temp,arr[i], gaf_ss.str());",
                "{i}  if(r{false}) {{ return r; }}",
                "{i}  c->{name}.push_back(temp);",
                "{i}}}"
            );
            for(const auto& line: lines)
            {
                const auto err = json_return_error(fmt::format("tried to read {} but value was not a array", m.name), "arr");
                sources->source.add
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
                "{i}c->{name} = std::make_shared<{type}>();"
                "{i}gaf_ss.str(\"\");"
                "{i}gaf_ss << gaf_path << \".{name}\";"
                "{i}{rv} r = ReadFromJsonValue(c->{name}.get(),iter->value, gaf_ss.str());"
                "{i}if(r{false})"
                "{i}{{"
                "{i}  c->{name}.reset();"
                "{i}  return r;"
                "{i}}}"
            );
            for(const auto& line: lines)
            {
                sources->source.add
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
                "{i}gaf_ss.str("");"
                "{i}gaf_ss << gaf_path << \".{name}\";"
                "{i}{rv} r = ReadFromJsonValue(&c->{name},iter->value, gaf_ss.str());"
                "{i}if(r{false})"
                "{i}{{"
                "{i}  return r;"
                "{i}}}"
            );
            for(const auto& line: lines)
            {
                sources->source.add
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
    sources->source.add(fmt::format("{} ReadFromJsonValue({}* c, const rapidjson::Value& value, const std::string& gaf_path) {{", json_return_value(), s.name));
    sources->source.add("  std::stringstream gaf_ss;");
    sources->source.add(fmt::format("  if(!value.IsObject()) {}", json_return_error(fmt::format("tried to read {} but value was not a object", s.name), "value")));
    sources->source.add("  rapidjson::Value::ConstMemberIterator iter;");
    for(const auto& m: s.members)
    {
        sources->source.add(fmt::format("  iter = value.FindMember(\"{}\");", m.name));
        sources->source.add("  if(iter != value.MemberEnd()) {");
        write_json_member(m, sources, "    ");
        sources->source.add("  }");
        if(m.missing_is_fail || m.is_optional)
        {
            sources->source.add("  else {");
            if(m.is_optional)
            {
                sources->source.add(fmt::format("    c->{}.reset();", m.name));
            }
            else
            {
                sources->source.add(fmt::format("    {}", json_return_error(fmt::format("missing {} in json object", m.name), "value")));
            }
            sources->source.add("  }");
        }
    }
    sources->source.add(fmt::format("  return {};", json_return_ok()));
    sources->source.add("}");
    sources->source.add("");
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
    sources->source.add(fmt::format("      if( ImGui::Button({}) )", opt.imgui_remove));
    sources->source.add("      {");
    sources->source.add("        delete_index = i;");
    sources->source.add("        please_delete = true;");
    sources->source.add("      }");
}


void write_single_imgui_member_to_source(const std::string& name, const std::string& var, const StandardType& t, Out* sources, const std::string& indent, const Member& m, bool add_delete, const ImguiOptions& opt)
{
    switch(t)
    {
    case StandardType::Int8:
        sources->source.add(fmt::format("{}ImGui::Edit({}, {});", indent, name, var));
        return;
    case StandardType::Int16:
        sources->source.add(fmt::format("{}ImGui::Edit({}, {});", indent, name, var));
        return;
    case StandardType::Int32:
        sources->source.add(fmt::format("{}ImGui::InputInt({}, {});", indent, name, var));
        return;
    case StandardType::Int64:
        sources->source.add(fmt::format("{}ImGui::Edit({}, {});", indent, name, var));
        return;
    case StandardType::Uint8:
        sources->source.add(fmt::format("{}ImGui::Edit({}, {});", indent, name, var));
        return;
    case StandardType::Uint16:
        sources->source.add(fmt::format("{}ImGui::Edit({}, {});", indent, name, var));
        return;
    case StandardType::Uint32:
        sources->source.add(fmt::format("{}ImGui::Edit({}, {});", indent, name, var));
        return;
    case StandardType::Uint64:
        sources->source.add(fmt::format("{}ImGui::Edit({}, {});", indent, name, var));
        return;
    case StandardType::Float:
        sources->source.add(fmt::format("{}ImGui::InputFloat({}, {});", indent, name, var));
        return;
    case StandardType::Double:
        sources->source.add(fmt::format("{}ImGui::InputDouble({}, {});", indent, name, var));
        return;
    case StandardType::Byte:
        sources->source.add(fmt::format("{}ImGui::Edit({}, {});", indent, name, var));
        return;
    case StandardType::Bool:
        sources->source.add(fmt::format("{}ImGui::Checkbox({}, {});", indent, name, var));
        return;
    case StandardType::String:
        sources->source.add(fmt::format("{}{{", indent));
        sources->source.add(fmt::format("{}  char gaf_temp[1024];", indent));
        sources->source.add(fmt::format("{}  strcpy(gaf_temp, ({})->c_str());", indent, var));
        sources->source.add(fmt::format("{}  if(ImGui::InputText({}, gaf_temp, 1024))", indent, name));
        sources->source.add(fmt::format("{}  {{", indent));
        sources->source.add(fmt::format("{}    *({}) = gaf_temp;", indent, var));
        sources->source.add(fmt::format("{}  }}", indent));
        sources->source.add(fmt::format("{}}}", indent));
        return;
    default:
        if(m.type_name.is_enum)
        {
            sources->source.add(fmt::format("{}RunImgui({}, {});", indent, var, name));
        }
        else
        {
            sources->source.add
            (
                fmt::format
                (
                    "{}if(ImGui::TreeNodeEx({}, ImGuiTreeNodeFlags_DefaultOpen{}))",
                    indent,
                    name,
                    add_delete ? "| ImGuiTreeNodeFlags_FramePadding" : ""
                )
            );
            sources->source.add(fmt::format("{}{{", indent));
            if(add_delete)
            {
                sources->source.add(fmt::format("{}  ImGui::SameLine();", indent));
                add_imgui_delete_button(m, sources, opt);
            }
            sources->source.add(fmt::format("{}  RunImgui({});", indent, var));
            sources->source.add(fmt::format("{}  ImGui::TreePop();", indent));
            sources->source.add(fmt::format("{}}}", indent));
            if(add_delete)
            {
                sources->source.add(fmt::format("{}else", indent));
                sources->source.add(fmt::format("{}{{", indent));
                sources->source.add(fmt::format("{}  ImGui::SameLine();", indent));
                add_imgui_delete_button(m, sources, opt);
                sources->source.add(fmt::format("{}}}", indent));
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
            sources->source.add(fmt::format("    if(c->{})", m.name));
            sources->source.add(fmt::format("    {"));
            write_single_imgui_member_to_source
            (
                fmt::format("\"{}\"", m.name),
                fmt::format("c->{}.get()", m.name),
                m.type_name.standard_type,
                sources, "      ", m, false, opt
            );
            sources->source.add(fmt::format("      if(ImGui::Button(\"Clear {}\")) {{ c->{name}.reset(); }}", m.name));
            sources->source.add("    }");
            sources->source.add("    else");
            sources->source.add("    {");
            sources->source.add(fmt::format("      if(ImGui::Button(\"Set {0}\")) {{ c->{0}.reset({1}); }}", m.name, determine_new_value(m)));
            sources->source.add("    }");
            sources->source.add("    ");
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
        sources->source.add(fmt::format("  if(ImGui::TreeNodeEx(\"{}\", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))", m.name));
        sources->source.add("  {");
        sources->source.add("    ImGui::SameLine();");
        sources->source.add(fmt::format("    if(ImGui::Button({}))", opt.imgui_add));
        sources->source.add("    {");
        sources->source.add(fmt::format("      c->{}.push_back({});", m.name, determine_pushback_value(m)));
        sources->source.add("    }");
        sources->source.add("    std::size_t delete_index = 0;");
        sources->source.add("    bool please_delete = false;");
        sources->source.add(fmt::format("    for(std::size_t i=0; i<c->{}.size(); i+= 1)", m.name));
        sources->source.add("    {");
        sources->source.add("      std::stringstream gaf_ss;");
        sources->source.add(fmt::format("      gaf_ss << \"{}[\" << i << \"]\";", m.name));
        sources->source.add("      ImGui::PushID(i);");
        write_single_imgui_member_to_source("gaf_ss.str().c_str()", fmt::format("&c->{}[i]", m.name), m.type_name.standard_type, sources, "      ", m, !short_version, opt);
        if(short_version)
        {
            sources->source.add(fmt::format("      ImGui::SameLine();"));
            add_imgui_delete_button(m, sources, opt);
        }
        sources->source.add("      ImGui::PopID();");
        sources->source.add("    }");
        sources->source.add("    if(please_delete)");
        sources->source.add("    {");
        sources->source.add(fmt::format("      c->{0}.erase(c->{0}.begin()+delete_index);", m.name));
        sources->source.add("    }");
        sources->source.add("    ImGui::TreePop();");
        sources->source.add("  }");
        sources->source.add("  else");
        sources->source.add("  {");
        sources->source.add("    ImGui::SameLine();");
        sources->source.add(fmt::format("    if(ImGui::Button({}))", opt.imgui_add));
        sources->source.add("    {");
        sources->source.add(fmt::format("      c->{}.push_back({});", m.name, determine_pushback_value(m)));
        sources->source.add("    }");
        sources->source.add("  }");
    }
}


void write_imgui_source_for_cpp(Out* sources, const Struct& s, const ImguiOptions& opt)
{
    sources->source.add(fmt::format("void RunImgui({}* c)", s.name));
    sources->source.add("{");
    for(const auto& m: s.members)
    {
        write_single_member_to_source(m, sources, opt);
    }
    sources->source.add("}");
    sources->source.add("");
}


void write_member_variables_for_cpp(Out* sources, const Struct& s)
{
    for(const auto& m: s.members)
    {
        // m.type_name.is_enum
        const auto type_name = m.type_name.name;
        if(m.is_optional)
        {
            sources->header.add(fmt::format("  std::shared_ptr<{}> {};", type_name, m.name));
        }
        else if(m.is_dynamic_array)
        {
            sources->header.add(fmt::format("  std::vector<{}> {};", type_name, m.name));
        }
        else
        {
            sources->header.add(fmt::format("  {} {};", type_name, m.name));
        }
    }
    sources->header.add("};");
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

    sources->header.add(fmt::format("  {}();", s.name));
    sources->header.add("");
    sources->source.add(fmt::format("{0}::{0}()", s.name));
    auto sep = ':';
    for(const auto& m: common_members)
    {
        auto default_value = *m.defaultvalue;
        if(m.type_name.is_enum)
        {
            default_value = fmt::format("{}::{}", m.type_name.name, *m.defaultvalue);
        }
        sources->source.add(fmt::format("  {} {}({})", sep, m.name, default_value));
        sep = ',';
    }
    sources->source.add("{}");
    sources->source.add("");
}


void iterate_enum(const Enum& e, Out* sources, bool prefix_prop=false)
{
    const auto prefix = prefix_prop ? fmt::format("{}_", e.name) : "";
    std::size_t index = 0;
    for(const auto& v: e.values)
    {
        const auto last = index == e.values.size();
        const auto comma = last ? "" : ",";
        sources->header.add(fmt::format("  {}{}{}", prefix, v, comma));
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
    sources->header.add(fmt::format("{} ReadFromJsonValue({}* c, const rapidjson::Value& value{});", json_return_value(), enum_type, arg));
    sources->source.add(fmt::format("{} ReadFromJsonValue({}* c, const rapidjson::Value& value{})", json_return_value(), enum_type, arg));
    sources->source.add("{");
    sources->source.add("  std::stringstream gaf_ss;");
    sources->source.add(fmt::format("  if(value.IsString()==false) {};", json_return_error(fmt::format("read value for {} was not a string", e.name), "value")));
    for(const auto& v: e.values)
    {
        sources->source.add
        (
            fmt::format
            (
                "  if(strcmp(value.GetString(), \"{v}\")==0) {{ *c = {p}{v}; return {ok};}}",
                fmt::arg("v", v),
                fmt::arg("p", value_prefix),
                fmt::arg("ok", json_return_ok())
            )
        );
    }
    sources->source.add(fmt::format("  {}", json_return_error(fmt::format("read string for {} was not valid", e.name), "value")));
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
    sources.header.add(fmt::format("#include \"gaf_{}.h\"", name));
    sources.header.add("");

    if(f.package_name.empty() == false)
    {
        sources.header.add(fmt::format("namespace {} {{", f.package_name));
        sources.header.add("");
    }

    if(f.typedefs.empty() == false)
    {
        for(const auto& s: f.typedefs)
        {
            sources.header.add(fmt::format("struct {};", s->name));
        }
        sources.header.add("");
    }

    for(const auto& e: f.enums)
    {
        add_enum_json_function(*e, &sources);
    }

    for(const auto& s: f.structs_defined)
    {
        write_json_source_for_cpp(&sources, *s);
        
        sources.header.add("");
        const auto arg = ", const std::string& gaf_path";
        sources.header.add(fmt::format("{} ReadFromJsonValue({}* c, const rapidjson::Value& value{});", json_return_value(), s->name, arg));
        sources.header.add("");
    }
    
    sources.header.add("std::string GafToString(const rapidjson::Value& val);");
    sources.source.add("std::string GafToString(const rapidjson::Value& val)");
    sources.source.add("{");
    sources.source.add("  if(val.IsNull()) { return \"Null\"; };");
    sources.source.add("  if(val.IsFalse()) { return \"False\"; };");
    sources.source.add("  if(val.IsTrue()) { return \"True\"; };");
    sources.source.add("  if(val.IsObject()) { return \"Object\"; };");
    sources.source.add("  if(val.IsArray()) { return \"Array\"; };");
    sources.source.add("  if(val.IsUint64()) { std::stringstream ss; ss << \"uint of \" << val.GetUint64(); return ss.str(); };");
    sources.source.add("  if(val.IsInt64()) { std::stringstream ss; ss << \"int of \" << val.GetInt64(); return ss.str(); };");
    sources.source.add("  if(val.IsDouble()) { std::stringstream ss; ss << \"double of \" << val.GetDouble(); return ss.str(); };");
    sources.source.add("  if(val.IsString()) { std::stringstream ss; ss << \"string of \" << val.GetString(); return ss.str(); };");
    sources.source.add("  return \"<unknown>\";");
    sources.source.add("}");
    sources.source.add("");

    // todo: remove this horrible function
    sources.header.add("std::string GafToString(int64_t val);");
    sources.source.add("std::string GafToString(int64_t val)");
    sources.source.add("{");
    sources.source.add("  std::stringstream ss;");
    sources.source.add("  ss << val;");
    sources.source.add("  return ss.str();");
    sources.source.add("}");
    sources.source.add("");

    sources.header.add("");

    if(f.package_name.empty() == false)
    {
        sources.header.add("}");
        sources.header.add("");
    }
    sources.header.add("");

    return sources;
}


Out generate_imgui(const File& f, const std::string& name, const ImguiOptions& opt)
{
    auto sources = Out{};

    sources.header.add("#pragma once");
    sources.header.add("");
    sources.header.add(fmt::format("#include \"gaf_{}.h\"", name));
    sources.header.add("");

    if(f.package_name.empty() == false)
    {
        sources.header.add(fmt::format("namespace {} {{", f.package_name));
        sources.header.add("");
    }

    if(f.typedefs.empty() == false)
    {
        for(const auto& s: f.typedefs)
        {
            sources.header.add(fmt::format("struct {};", s->name));
        }
        sources.header.add("");
    }

    for(const auto& e: f.enums)
    {
        sources.header.add(fmt::format("const char* ToString({} en);", name));
        sources.source.add(fmt::format("const char* ToString({} en)", name));
        sources.source.add("{");
        for(const auto& v: e->values)
        {
            sources.source.add(fmt::format("  if(en == {0}{1}) {{ return \"{1}\"; }}", get_value_prefix_opt(*e), v));
        }
        sources.source.add("  return \"<invalid value>\";");
        sources.source.add("}");

        sources.header.add(fmt::format("void RunImgui({}* en, const char* label);", e->name));
        sources.source.add(fmt::format("void RunImgui({}* en, const char* label)", e->name));
        sources.source.add("{");
        sources.source.add("  if(ImGui::BeginCombo(label, ToString(*en)))");
        sources.source.add("  {");
        for(const auto& v: e->values)
        {
            sources.source.add(fmt::format("    if(ImGui::Selectable(\"{0}\", *en == {1}{0})) {{ *en = {prefix}{val}; }}", v, get_value_prefix_opt(*e)));
        }
        sources.source.add("    ImGui::EndCombo();");
        sources.source.add("  }");
        sources.source.add("}");
        sources.header.add("");
        sources.source.add("");
    }
    
    for(const auto& s: f.structs_defined)
    {
        write_imgui_source_for_cpp(&sources, *s, opt);

        sources.header.add("");
        sources.header.add(fmt::format("void RunImgui({}* c);", s->name));
    }
    
    if(f.package_name.empty() == false)
    {
        sources.header.add("}");
        sources.header.add("");
    }
    sources.header.add("");

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

    sources.header.add("#pragma once");
    sources.header.add("");

    bool added_include = false;
    if(default_types.empty() == false)
    {
        added_include = true;
        sources.header.add("#include <cstdint>");
    }
    if(has_string)
    {
        added_include = true;
        sources.header.add("#include <string>");
    }
        
    if(has_dynamic_arrays)
    {
        added_include = true;
        sources.header.add("#include <vector>");
    }
    if(has_optional)
    {
        added_include = true;
        sources.header.add("#include <memory>");
    }
    if(added_include)
    {
        sources.header.add("");
    }

    if(f.package_name.empty() == false)
    {
        sources.header.add(fmt::format("namespace {} {{", f.package_name));
        sources.header.add("");
    }

    if(default_types.empty() == false)
    {
        for(const auto& t: default_types)
        {
            sources.header.add(fmt::format("typedef {} {};", t.get_cpp_type(), t.name));
        }
        sources.header.add("");
    }

    if(f.typedefs.empty() == false)
    {
        for(const auto& s: f.typedefs)
        {
            sources.header.add(fmt::format("struct {};", s->name));
        }
        sources.header.add("");
    }

    for(const auto& e: f.enums)
    {
        sources.header.add(fmt::format("enum class {} {{", e->name));
        iterate_enum(*e, &sources);
        sources.header.add(fmt::format("}}; // enum {}", e->name));

        sources.header.add("");
    }

    for(const auto& s: f.structs_defined)
    {
        sources.header.add(fmt::format("struct {} {{", s->name));
        write_default_constructor_for_cpp(*s, &sources);

        write_member_variables_for_cpp(&sources, *s);
        sources.header.add("");
    }

    if(f.package_name.empty() == false)
    {
        sources.header.add("}");
        sources.header.add("");
    }
    sources.header.add("");

    return sources;
}

std::string get_file_path(const std::string& folder, const std::string& name)
{
    const auto r = std::filesystem::path{folder} / std::filesystem::path{name};
    const auto a = std::filesystem::absolute(r);
    return a.string();
}

void write_lines(const Lines& lines, Writer* writer, const std::string& path)
{
    if(auto out = writer->open(path); out != nullptr)
    {
        for(const auto& s: lines.lines)
        {
            out->write(s);
        }
    }
}

Lines complete_source(const Lines& source, const std::string& name, const std::string& prefix, const std::string& package_name, const std::vector<std::string>& includes)
{
    Lines ret;

    ret.add(fmt::format("#include \"{}.h\"", prefix + name));
    ret.add("");
    for(const auto& inc: includes)
    {
        ret.add(fmt::format("#include {}", inc));
    }
    ret.add("");

    if(package_name.empty() == false)
    {
        ret.add(fmt::format("namespace {} {{", package_name));
        ret.add("");
    }

    for(const auto& s: source.lines)
    {
        ret.add(s);
    }

    if(package_name.empty() == false)
    {
        ret.add("}");
        ret.add("");
    }

    return ret;
}

void write_cpp(Out* sources, Writer* writer, const std::string& out_dir, const std::string& name, const std::string& prefix, const std::string& package_name, const std::vector<std::string>& includes)
{
    write_lines(sources->header, writer, get_file_path(out_dir, prefix + name + ".h"));
    write_lines(complete_source(sources->source, name, prefix, package_name, includes), writer, get_file_path(out_dir, prefix + name + ".cc"));
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
            std::cerr << "invalid option " << c << "";
            return -42;
        }
    }
    
    
    auto out = generate_imgui(file, name, ImguiOptions{imgui_add, imgui_remove});
    write_cpp(&out, writer, output_folder, name, "gaf_imgui_", file.package_name, imgui_headers);
    
    return 0;
}
