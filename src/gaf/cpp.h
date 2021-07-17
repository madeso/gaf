#pragma once

#include "gaf/types.h"

struct CppPlugin : Plugin
{
    ~CppPlugin() = default;
    std::string get_name() override;
    int run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args, const std::string& name) override;
};

struct RapidJsonPlugin : Plugin
{
    ~RapidJsonPlugin() = default;
    std::string get_name() override;
    int run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args, const std::string& name) override;
};

struct ImguiPlugin : Plugin
{
    ~ImguiPlugin() = default;
    std::string get_name() override;
    int run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args, const std::string& name) override;
};
