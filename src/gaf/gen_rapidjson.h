#pragma once

#include "gaf/types.h"

struct RapidJsonPlugin : Plugin
{
    ~RapidJsonPlugin() = default;
    std::string get_name() override;
    int run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args,
                   const std::string& name) override;
};
