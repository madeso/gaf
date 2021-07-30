#pragma once

#include "gaf/types.h"

struct PugiXmlPlugin : Plugin
{
    ~PugiXmlPlugin() = default;

    std::string get_name() override;
    int run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args,
                   const std::string& name) override;
};
