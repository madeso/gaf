#include "gaf/gen_imgui.h"

#include <iostream>

#include "fmt/format.h"

#include "gaf/generator.h"
#include "gaf/args.h"

namespace imgui
{
    struct ImguiOptions
    {
        std::string imgui_add = "\"+\"";
        std::string imgui_remove = "\"-\"";
    };

    std::string determine_pushback_value(const Member& m)
    {
        const auto t = m.type_name;
        if (t.standard_type == StandardType::String)
        {
            return "\"\"";
        }
        auto tl = TypeList{};
        tl.add_default_types();
        if (tl.is_valid_type(t.name))
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
        sources->source.addf("if( ImGui::Button({}) )", opt.imgui_remove);
        sources->source.add("{");
        sources->source.add("delete_index = i;");
        sources->source.add("please_delete = true;");
        sources->source.add("}");
    }

    void write_single_imgui_member_to_source(const std::string& name, const std::string& var,
                                             const StandardType& t, Out* sources, const Member& m,
                                             bool add_delete, const ImguiOptions& opt)
    {
        switch (t)
        {
        case StandardType::Int8:
            sources->source.addf("ImGui::Edit({}, {});", name, var);
            return;
        case StandardType::Int16:
            sources->source.addf("ImGui::Edit({}, {});", name, var);
            return;
        case StandardType::Int32:
            sources->source.addf("ImGui::InputInt({}, {});", name, var);
            return;
        case StandardType::Int64:
            sources->source.addf("ImGui::Edit({}, {});", name, var);
            return;
        case StandardType::Uint8:
            sources->source.addf("ImGui::Edit({}, {});", name, var);
            return;
        case StandardType::Uint16:
            sources->source.addf("ImGui::Edit({}, {});", name, var);
            return;
        case StandardType::Uint32:
            sources->source.addf("ImGui::Edit({}, {});", name, var);
            return;
        case StandardType::Uint64:
            sources->source.addf("ImGui::Edit({}, {});", name, var);
            return;
        case StandardType::Float:
            sources->source.addf("ImGui::InputFloat({}, {});", name, var);
            return;
        case StandardType::Double:
            sources->source.addf("ImGui::InputDouble({}, {});", name, var);
            return;
        case StandardType::Bool:
            sources->source.addf("ImGui::Checkbox({}, {});", name, var);
            return;
        case StandardType::String:
            sources->source.add("{");
            sources->source.add("char gaf_temp[1024];");
            sources->source.addf("strcpy(gaf_temp, ({})->c_str());", var);
            sources->source.addf("if(ImGui::InputText({}, gaf_temp, 1024))", name);
            sources->source.add("{");
            sources->source.addf("*({}) = gaf_temp;", var);
            sources->source.add("}");
            sources->source.add("}");
            return;
        default:
            if (m.type_name.is_enum)
            {
                sources->source.addf("RunImgui({}, {});", var, name);
            }
            else
            {
                sources->source.add(
                    fmt::format("if(ImGui::TreeNodeEx({}, ImGuiTreeNodeFlags_DefaultOpen{}))", name,
                                add_delete ? "| ImGuiTreeNodeFlags_FramePadding" : ""));
                sources->source.add("{");
                if (add_delete)
                {
                    sources->source.add("ImGui::SameLine();");
                    add_imgui_delete_button(m, sources, opt);
                }
                sources->source.addf("RunImgui({});", var);
                sources->source.add("ImGui::TreePop();");
                sources->source.add("}");
                if (add_delete)
                {
                    sources->source.add("else");
                    sources->source.add("{");
                    sources->source.add("ImGui::SameLine();");
                    add_imgui_delete_button(m, sources, opt);
                    sources->source.add("}");
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
        if (tl.is_valid_type(t.name) && t.standard_type != StandardType::String)
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
        if (m.is_dynamic_array == false)
        {
            if (m.is_optional)
            {
                sources->source.addf("if(c->{})", m.name);
                sources->source.add("{");
                write_single_imgui_member_to_source(fmt::format("\"{}\"", m.name),
                                                    fmt::format("c->{}.get()", m.name),
                                                    m.type_name.standard_type, sources, m, false, opt);
                sources->source.addf("if(ImGui::Button(\"Clear {0}\")) {{ c->{0}.reset(); }}", m.name);
                sources->source.add("}");
                sources->source.add("else");
                sources->source.add("{");
                sources->source.addf("if(ImGui::Button(\"Set {0}\")) {{ c->{0}.reset({1}); }}", m.name,
                                     determine_new_value(m));
                sources->source.add("}");
                sources->source.add("");
            }
            else
            {
                write_single_imgui_member_to_source(fmt::format("\"{}\"", m.name),
                                                    fmt::format("&c->{}", m.name),
                                                    m.type_name.standard_type, sources, m, false, opt);
            }
        }
        else
        {
            const auto short_version =
                m.type_name.standard_type != StandardType::INVALID || m.type_name.is_enum;
            sources->source.addf(
                "if(ImGui::TreeNodeEx(\"{}\", ImGuiTreeNodeFlags_DefaultOpen | "
                "ImGuiTreeNodeFlags_FramePadding))",
                m.name);
            sources->source.add("{");
            sources->source.add("ImGui::SameLine();");
            sources->source.addf("if(ImGui::Button({}))", opt.imgui_add);
            sources->source.add("{");
            sources->source.addf("c->{}.push_back({});", m.name, determine_pushback_value(m));
            sources->source.add("}");
            sources->source.add("std::size_t delete_index = 0;");
            sources->source.add("bool please_delete = false;");
            sources->source.addf("for(std::size_t i=0; i<c->{}.size(); i+= 1)", m.name);
            sources->source.add("{");
            sources->source.add("std::stringstream gaf_ss;");
            sources->source.addf("gaf_ss << \"{}[\" << i << \"]\";", m.name);
            sources->source.add("ImGui::PushID(i);");
            write_single_imgui_member_to_source("gaf_ss.str().c_str()", fmt::format("&c->{}[i]", m.name),
                                                m.type_name.standard_type, sources, m, !short_version,
                                                opt);
            if (short_version)
            {
                sources->source.addf("ImGui::SameLine();");
                add_imgui_delete_button(m, sources, opt);
            }
            sources->source.add("ImGui::PopID();");
            sources->source.add("}");
            sources->source.add("if(please_delete)");
            sources->source.add("{");
            sources->source.addf("c->{0}.erase(c->{0}.begin()+delete_index);", m.name);
            sources->source.add("}");
            sources->source.add("ImGui::TreePop();");
            sources->source.add("}");
            sources->source.add("else");
            sources->source.add("{");
            sources->source.add("ImGui::SameLine();");
            sources->source.addf("if(ImGui::Button({}))", opt.imgui_add);
            sources->source.add("{");
            sources->source.addf("c->{}.push_back({});", m.name, determine_pushback_value(m));
            sources->source.add("}");
            sources->source.add("}");
        }
    }

    std::string get_value_prefix_opt(const Enum& e)
    {
        return fmt::format("{}::", e.name);
    }

    void write_imgui_source_for_cpp(Out* sources, const Struct& s, const ImguiOptions& opt)
    {
        sources->source.addf("void RunImgui({}* c)", s.name);
        sources->source.add("{");
        for (const auto& m : s.members)
        {
            write_single_member_to_source(m, sources, opt);
        }
        sources->source.add("}");
        sources->source.add("");
    }

    Out generate_imgui(const File& f, const std::string& name, const ImguiOptions& opt,
                       const std::vector<std::string>& headers)
    {
        auto sources = Out{};

        sources.header.add("#pragma once");
        sources.header.add("");
        sources.header.addf("#include \"gaf_{}.h\"", name);
        sources.header.add("");

        for (const auto& h : headers)
        {
            sources.source.addf("#include {}", h);
        }
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
            sources.header.addf("const char* ToString({} en);", name);
            sources.source.addf("const char* ToString({} en)", name);
            sources.source.add("{");
            for (const auto& v : e->values)
            {
                sources.source.addf("if(en == {0}{1}) {{ return \"{1}\"; }}", get_value_prefix_opt(*e),
                                    v);
            }
            sources.source.add("return \"<invalid value>\";");
            sources.source.add("}");

            sources.header.addf("void RunImgui({}* en, const char* label);", e->name);
            sources.source.addf("void RunImgui({}* en, const char* label)", e->name);
            sources.source.add("{");
            sources.source.add("if(ImGui::BeginCombo(label, ToString(*en)))");
            sources.source.add("{");
            for (const auto& v : e->values)
            {
                sources.source.addf("if(ImGui::Selectable(\"{0}\", *en == {1}{0})) {{ *en = {1}{0}; }}",
                                    v, get_value_prefix_opt(*e));
            }
            sources.source.add("ImGui::EndCombo();");
            sources.source.add("}");
            sources.source.add("}");
            sources.add("");
        }

        for (const auto& s : f.structs)
        {
            write_imgui_source_for_cpp(&sources, *s, opt);

            sources.header.add("");
            sources.header.addf("void RunImgui({}* c);", s->name);
        }

        if (f.package_name.empty() == false)
        {
            sources.header.add("}");
            sources.header.add("");
        }

        return sources;
    }

}

std::string ImguiPlugin::get_name()
{
    return "imgui";
}

int ImguiPlugin::run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args,
                            const std::string& name)
{
    std::vector<std::string> imgui_headers = {"\"imgui.h\""};
    std::string imgui_add = "\"Add\"";
    std::string imgui_remove = "\"Remove\"";

    while (args.has_more())
    {
        const auto c = args.read();
        if (c == "--imgui-add")
        {
            imgui_add = args.read();
        }
        else if (c == "--imgui-remove")
        {
            imgui_remove = args.read();
        }
        else if (c == "--imgui-headers")
        {
            imgui_headers.clear();
            while (is_option(args.peek()) == false)
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

    auto out =
        imgui::generate_imgui(file, name, imgui::ImguiOptions{imgui_add, imgui_remove}, imgui_headers);
    write_cpp(&out, writer, output_folder, name, "gaf_imgui_");

    return 0;
}
