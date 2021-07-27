#include "gaf/gen_cpp.h"
#include "gaf/generator.h"
#include "gaf/args.h"

#include "fmt/format.h"


namespace cpp
{

    void iterate_enum(const Enum& e, Out* sources, bool prefix_prop = false)
    {
        const auto prefix = prefix_prop ? fmt::format("{}_", e.name) : "";
        std::size_t index = 0;
        for (const auto& v : e.values)
        {
            const auto last = index == e.values.size();
            const auto comma = last ? "" : ",";
            sources->header.add(fmt::format("{}{}{}", prefix, v, comma));
            index += 1;
        }
    }

    void write_member_variables_for_cpp(Out* sources, const Struct& s)
    {
        for (const auto& m : s.members)
        {
            // m.type_name.is_enum
            const auto type_name = m.type_name.get_cpp_type();
            if (m.is_optional)
            {
                sources->header.add(fmt::format("std::shared_ptr<{}> {};", type_name, m.name));
            }
            else if (m.is_dynamic_array)
            {
                sources->header.add(fmt::format("std::vector<{}> {};", type_name, m.name));
            }
            else
            {
                if (m.defaultvalue.has_value())
                {
                    auto default_value = *m.defaultvalue;
                    if (m.type_name.is_enum)
                    {
                        default_value = fmt::format("{}::{}", m.type_name.name, *m.defaultvalue);
                    }
                    sources->header.add(fmt::format("{} {} = {};", type_name, m.name, default_value));
                }
                else
                {
                    sources->header.add(fmt::format("{} {};", type_name, m.name));
                }
            }
        }
    }


    std::set<std::string> get_headers_types(const File& f)
    {
        std::set<std::string> r;
        const auto include = [&r](const std::string& i)
        {
            r.emplace(i);
        };

        for (const auto& s : f.structs)
        {
            for (const auto& m : s->members)
            {
                const auto& t = m.type_name;

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
                    include("<cstdint>");
                    break;

                case StandardType::String:
                    include("<string>");
                    break;
                default:
                    break;
                }

                if (m.is_dynamic_array)
                {
                    include("<vector>");
                }
                if (m.is_optional)
                {
                    include("<memory>");
                }
            }
        }
        return r;
    }

    Out generate_cpp(const File& f)
    {
        auto sources = Out{};

        // get all standard types used for typedefing later on...
        const auto headers = get_headers_types(f);

        sources.header.add("#pragma once");
        sources.header.add("");

        if (headers.empty() == false)
        {
            for (const auto& header : headers)
            {
                sources.header.add(fmt::format("#include {}", header));
            }
            sources.header.add("");
        }

        if (f.package_name.empty() == false)
        {
            sources.header.add(fmt::format("namespace {} {{", f.package_name));
            sources.header.add("");
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
            sources.header.add(fmt::format("enum class {} {{", e->name));
            iterate_enum(*e, &sources);
            sources.header.add("};");

            sources.header.add("");
        }

        for (const auto& s : f.structs)
        {
            sources.header.add(fmt::format("struct {} {{", s->name));

            write_member_variables_for_cpp(&sources, *s);
            sources.header.add("};");
            sources.header.add("");
        }

        if (f.package_name.empty() == false)
        {
            sources.header.add("}");
            sources.header.add("");
        }
        sources.header.add("");

        return sources;
    }

}


std::string CppPlugin::get_name()
{
    return "cpp";
}

int CppPlugin::run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args, const std::string& name)
{
    if (auto r = no_arguments(args); r != 0)
    {
        return r;
    }

    auto out = cpp::generate_cpp(file);
    write_cpp(&out, writer, output_folder, name, "gaf_", file.package_name, {"<limits>"});

    return 0;
}
