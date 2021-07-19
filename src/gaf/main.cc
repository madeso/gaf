// GAme Format Parser

#include <iostream>
#include <fstream>
#include <filesystem>

#include "gaf/args.h"
#include "gaf/types.h"
#include "gaf/parse.h"
#include "gaf/cpp.h"

// from gaf_cpp import CppPlugin, RapidJsonPlugin, ImguiPlugin
// from gaf_parse import CharFile, read_several_structs, ParseError
// from gaf_types import Plugin

using Plugins = std::vector<std::shared_ptr<Plugin>>;

void write_errors(const CharFile& f)
{
    for(const auto& e: f.errors)
    {
        std::cerr << e << "\n";
    }
}

int on_display_command(Args& args, const Plugins&)
{
    const auto input = args.read();

    if(input == "")
    {
        std::cerr << "missing file\n";
        return -42;
    }

    if(auto r = no_arguments(args); r != 0)
    {
        return r;
    }

    while(args.has_more())
    {
        const auto r = args.read();
        std::cerr << "invalid argument " << r << "\n";
        return -42;
    }
    
    auto file = CharFile{input};
    auto parsed_file = read_several_structs(&file);
    if(parsed_file == nullptr)
    {
        std::cerr << "unable to read file " << input << "\n";
        write_errors(file);
        return -42;
    }
    
    std::cout << *parsed_file;
    return 0;
}

struct ConsoleFileOut : FileOut
{
    std::string file;

    explicit ConsoleFileOut(const std::string& f) : file(f) {}

    void write(const std::string& line) override
    {
        std::cout << "'"<< file << "': " << line;
    }
};

struct ConsoleWriter : Writer
{
    std::unique_ptr<FileOut> open(const std::string& f) override
    {
        return std::make_unique<ConsoleFileOut>(f);
    }
};

struct FileFileOut : FileOut
{
    std::ofstream ff;

    explicit FileFileOut(const std::string& p)
        : ff(p.c_str())
    {
    }

    void write(const std::string& line) override
    {
        ff << line;
    }
};

struct FileWriter : Writer
{
    std::unique_ptr<FileOut> open(const std::string& f) override
    {
        return std::make_unique<FileFileOut>(f);
    }
};

int on_generate_command(Args& args, const Plugins& plugins)
{
    auto input = args.read();
    auto folder = args.read();
    auto command = args.read();

    bool debug = false;
    if(args.peek() == "--debug")
    {
        debug = true;
        args.read();
    }

    auto file_writer = FileWriter{};
    auto console_writer = ConsoleWriter{};

    Writer* writer = &file_writer;
    if(debug) writer = &console_writer;

    if(input == "")
    {
        std::cerr << "missing file\n";
        return -42;
    }

    if(folder == "")
    {
        std::cerr << "missing folder\n";
        return -42;
    }

    if(command == "")
    {
        std::cerr << "missing command\n";
        return -42;
    }

    auto file = CharFile{input};
    const auto parsed_file = read_several_structs(&file);
    if(parsed_file == nullptr)
    {
        std::cerr << "unable to read file " << input << "\n";
        write_errors(file);
        return -42;
    }

    const auto name = std::filesystem::path(file.name).stem();

    for(auto& plugin: plugins)
    {
        if(command == plugin->get_name())
        {
            return plugin->run_plugin(*parsed_file, writer, folder, args, name);
        }
    }

    std::cerr << "unknown plugin " << command << "\n";
    std::cout << "Valid plugins:\n";
    for(const auto& p: plugins)
    {
        std::cout << " - " << p->get_name() << "\n";
    }
    return -42;
}


int run_main(Args& args, const Plugins& plugins)
{
    if(args.peek() == "")
    {
        std::cerr << args.app << " <command>\n";
        return -42;
    }

    const auto command = args.read();

    if(command == "display")
    {
        on_display_command(args, plugins);
    }
    else if(command == "generate")
    {
        return on_generate_command(args, plugins);
    }
    else
    {
        std::cerr << "invalid command " << command << ", either display or generate\n";
        return -42;
    }
    
    return 0;
}


int main(int argc, char *argv[])
{
    auto args = Args{argv[0]};
    for(int i=1; i<argc; i+=1)
    {
        args.args.emplace_back(argv[i]);
    }

    return run_main
    (
        args,
        {
            std::make_shared<CppPlugin>(),
            std::make_shared<RapidJsonPlugin>(),
            std::make_shared<ImguiPlugin>()
        }
    );
}
