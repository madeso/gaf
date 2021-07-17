#pragma once

#include <memory>
#include <vector>
#include <string>

struct File;


/** Representation of a file loaded to memory.
 */
struct CharFile
{
    std::string name;
    std::string data;
    std::vector<std::string> errors;

    std::size_t index = 0;
    std::size_t line = 1;

    CharFile(const std::string& file);

    // read a single character from the file
    char read();

    // peek ahead of the current position and return it's character or 0
    char peek(int count) const;

    // raise a parse error at the current line and byte offset
    void report_error(const std::string& message);
};

std::shared_ptr<File> read_several_structs(CharFile* f);
