#include "gaf/types.h"

#include <optional>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <cassert>

#include "fmt/format.h"

#include "gaf/parse.h"


std::string read_file_to_string(const std::string& path)
{
    std::ifstream t(path.c_str());
    return std::string((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
}



CharFile::CharFile(const std::string& file)
    : name(file)
    , data(read_file_to_string(file))
{
}


char CharFile::read()
{
    const auto c = data[index];
    index += 1;
    if(c == '\n')
    {
        line += 1;
    }
    return c;
}


char CharFile::peek(int count) const
{
    if(index + count >= data.size())
    {
        return 0;
    }
    return data[index + count];
}

void CharFile::report_error(const std::string& message)
{
    std::ostringstream ss;
    ss << name << "(" << line << "): " << message;
    errors.emplace_back(ss.str());
}


// util function to read a char from a file
char read_char(CharFile* f)
{
    return f->read();
}

// util function to peeek ahead in a file
char peek_char(CharFile* f, int count=0)
{
    return f->peek(count);
}

// function to check if the character is a space or not
bool is_space(char ch)
{
    switch(ch)
    {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
        return true;
    default:
        return false;
    }
}

// util function to chechk if the optional character is part of a identifier or not
bool is_ident(bool first, char ch)
{
    if(ch == 0)
    {
        return false;
    }
    if('a' <= ch && ch <= 'z')
    {
        return true;
    }
    if('A' <= ch && ch <= 'Z')
    {
        return true;
    }
    if(ch == '_')
    {
        return true;
    }
    if(first == false)
    {
        if('0' <= ch && ch <= '9')
        {
            return true;
        }
    }
    return false;
}

// checks if a character is a number
bool is_number(char ch)
{
    return '0' <= ch && ch <= '9';
}

// skips white space in a file
void read_white_spaces(CharFile* f)
{
    while(is_space(peek_char(f)))
    {
        read_char(f);
    }
}


// skips white space or comments
void read_spaces(CharFile* f)
{
    while(true)
    {
        read_white_spaces(f);
        if(peek_char(f, 0) == '/' && peek_char(f, 1) == '/')
        {
            while(peek_char(f) != '\n')
            {
                read_char(f);
            }
        }
        else if(peek_char(f, 0) == '/' && peek_char(f, 1) == '*')
        {
            auto slash = read_char(f);
            auto star = read_char(f);
            assert(slash == '/');
            assert(star == '*');
            while(peek_char(f, 0) != '*' || peek_char(f, 1) != '/')
            {
                read_char(f);
            }
            star = read_char(f);
            slash = read_char(f);
            assert(slash == '/');
            assert(star == '*');
        }
        else
        {
            return;
        }
    }
}

// reads a ident from file, skips spaces before
std::string read_ident(CharFile* f)
{
    read_spaces(f);
    std::string ident;
    bool first = true;
    while(is_ident(first, peek_char(f)))
    {
        ident += read_char(f);
        first = false;
    }
    if(ident.empty())
    {
        f->report_error(fmt::format("Expecting ident but found {}", peek_char(f)));
        return "no_identifier";
    }
    return ident;
}

// skips spaces and expects a single char from the file
void read_single_char(CharFile* f, char ch)
{
    read_spaces(f);
    const auto r = read_char(f);
    if(r != ch)
    {
        f->report_error(fmt::format("expecting char {}, but found {}", ch, r));
    }
}

// read a number (typical integer) like 0 42 or 123
std::string read_number(CharFile* f)
{
    std::string ret;
    while(is_number(peek_char(f)))
    {
        ret += read_char(f);
    }
    if(ret.empty() == 0)
    {
        f->report_error(fmt::format("Expected number, found {}", peek_char(f)));
        return "0";
    }
    return ret;
}

// read a integer
std::string read_default_value_int(CharFile* f)
{
    return read_number(f);
}

// read a double
std::string read_default_value_double(CharFile* f)
{
    const auto dec = read_number(f);
    read_single_char(f, '.');
    const auto frac = read_number(f);
    return fmt::format("{}.{}", dec, frac);
}

// read a string
std::string read_default_value_string(CharFile* f)
{
    read_single_char(f, '\"');
    std::string r = "\"";
    while(peek_char(f) != '\"')
    {
        const auto ch = read_char(f);
        r += ch;
        if(ch == '\\')
        {
            r += read_char(f);
        }
    }

    read_single_char(f, '"');
    r += '"';
    return r;
}


// read default values for a property
std::string read_default_value(CharFile* f, const Type& t, File* fi)
{
    read_spaces(f);

    auto p = peek_char(f);
    if(is_ident(true, p))
    {
        const auto ident = read_ident(f);

        if(t.standard_type == StandardType::Bool)
        {
            if(ident == "true")
            {
                return "true";
            }
            else if(ident == "false")
            {
                return "false";
            }
            else
            {
                f->report_error(fmt::format("{} is not a valid bool", ident));
                return "false";
            }
        }
        if(t.is_enum)
        {
            auto e = fi->find_enum(t.name);
            if(e == nullptr)
            {
                f->report_error(fmt::format("BUG: failed to find enum {} while loooking up {}", ident, t.name));
                return "";
            }
            if(e->is_value(ident))
            {
                return ident;
            }
            else
            {
                f->report_error(fmt::format("{} is not a valid enum value of {}", ident, t.name));
                return "";
            }
        }
        auto c = fi->find_constant(ident, t);
        if(c == nullptr)
        {
            f->report_error(fmt::format("failed to find constant named {} with a type {}", ident, t.name));
            return "";
        }
        return c->value;
    }
    if(t.standard_type == StandardType::Int8)
    {
        return read_default_value_int(f);
    }
    if(t.standard_type == StandardType::Int16)
    {
        return read_default_value_int(f);
    }
    if(t.standard_type == StandardType::Int32)
    {
        return read_default_value_int(f);
    }
    if(t.standard_type == StandardType::Int64)
    {
        return read_default_value_int(f);
    }
    if(t.standard_type == StandardType::Float)
    {
        const auto fl = read_default_value_double(f);
        read_single_char(f, 'f');
        return fl;
    }
    if(t.standard_type == StandardType::String)
    {
        return read_default_value_string(f);
    }
    if(t.standard_type == StandardType::Double)
    {
        return read_default_value_double(f);
    }
    if(t.standard_type == StandardType::Byte)
    {
        f->report_error("default value for byte is not yet supported");
        return "";
    }
    return "";
}

// read a single struct
std::shared_ptr<Struct> read_struct(CharFile* f, TypeList* type_list, File* fi)
{
    const auto struct_name = read_ident(f);

    auto r = fi->find_struct(struct_name);
    if(r == nullptr)
    {
        r = std::make_shared<Struct>(struct_name);
        type_list->add_type({StandardType::INVALID, struct_name, false});
    }

    read_spaces(f);
    auto ch = peek_char(f);

    if(ch == ';')
    {
        read_char(f);
    }
    else if(ch == '{')
    {
        read_char(f);
        if(r->is_defined)
        {
            f->report_error(fmt::format("structs {} has already been defined", struct_name));
        }
        r->is_defined = true;
        while(peek_char(f) != '}')
        {
            const auto ty = read_ident(f);

            auto is_optional = false;
            read_spaces(f);
            ch = peek_char(f);
            if(ch == '?')
            {
                is_optional = true;
                read_char(f);
                read_spaces(f);
                ch = peek_char(f);
            }
            const auto name = read_ident(f);
            if(type_list->is_valid_type(ty) == false)
            {
                f->report_error(fmt::format("Invalid type {} for member {}.{}", ty, struct_name, name));
            }
            const auto valid_type = type_list->is_valid_type(ty) ? type_list->get_type(ty) : Type::create_error_type();
            const auto s = fi->find_struct(ty);
            if(s != nullptr && is_optional == false)
            {
                if(s->is_defined == false)
                {
                    f->report_error(fmt::format("Struct {} is not defined yet for {}.{}. Define or use optional", ty, struct_name, name));
                }
            }
            auto mem = Member{name, valid_type};
            mem.is_optional = is_optional;

            read_spaces(f);
            ch = peek_char(f);

            if(is_optional == false)
            {
                if(ch == '[')
                {
                    read_char(f);
                    read_spaces(f);
                    read_single_char(f, ']');
                    mem.is_dynamic_array = true;
                    mem.defaultvalue = {};

                    read_spaces(f);
                    ch = peek_char(f);
                }
                if(ch == '?')
                {
                    read_char(f);
                    read_spaces(f);
                    ch = peek_char(f);
                    mem.missing_is_fail = false;
                }
            }
            if(ch == '=')
            {
                if(is_default_type(ty)==false && valid_type.is_enum == false)
                {
                    f->report_error("structs cant have default values yet");
                }
                if(mem.is_dynamic_array)
                {
                    f->report_error("dynamic arrays cant have default values yet");
                }
                read_char(f);
                mem.defaultvalue = read_default_value(f, valid_type, fi);
            }
            read_spaces(f);
            read_single_char(f, ';');

            r->members.emplace_back(mem);
            read_spaces(f);
        }
        read_single_char(f, '}');
    }

    return r;
}


// read a single enum
std::shared_ptr<Enum> read_enum(CharFile* f, TypeList* type_list)
{
    const auto enum_name = read_ident(f);
    auto e = std::make_shared<Enum>(enum_name);

    read_spaces(f);
    if(peek_char(f) == ':')
    {
        read_char(f);
        const auto type_name = read_ident(f);
        if(type_list->is_valid_type(type_name) == false)
        {
            f->report_error(fmt::format("{} has a invalid type for size type: {}", enum_name, type_name));
        }
        const auto t = type_list->get_type(type_name);
        if(t.is_int == false)
        {
            f->report_error(fmt::format("{} has a non-int for size type: {}", enum_name, type_name));
        }
        e->type = t;
    }

    read_spaces(f);
    read_single_char(f, '{');
    while(peek_char(f) != '}')
    {
        const auto name = read_ident(f);

        if(e->is_value(name))
        {
            f->report_error(fmt::format("{} already specifed in {}", name, enum_name));
        }

        e->add_value(name);

        read_spaces(f);
        auto ch = peek_char(f);

        if(ch == ',')
        {
            read_char(f);
            read_spaces(f);
        }
    }
    read_single_char(f, '}');

    type_list->add_type({StandardType::INVALID, enum_name, false, e->values[0], true});

    return e;
}

// parse a file
std::shared_ptr<File> read_several_structs(CharFile* f)
{
    auto file = std::make_shared<File>();
    read_spaces(f);
    auto type_list = TypeList{};
    type_list.add_default_types();
    while(peek_char(f) != 0)
    {
        const auto keyword = read_ident(f);
        if(keyword == "struct")
        {
            auto s = read_struct(f, &type_list, file.get());
            if(file->find_struct(s->name) == nullptr)
            {
                file->structs.emplace_back(s);
            }

            if(s->is_defined)
            {
                file->structs_defined.emplace_back(s);
            }
            else
            {
                file->typedefs.emplace_back(s);
            }
        }
        else if(keyword == "enum")
        {
            auto e = read_enum(f, &type_list);
            file->enums.emplace_back(e);
        }
        else if(keyword == "const")
        {
            const auto ty = read_ident(f);
            const auto name = read_ident(f);
            read_spaces(f);
            read_single_char(f, '=');
            if(type_list.is_valid_type(ty) == false)
            {
                f->report_error(fmt::format("Invalid type {} for const {}", ty, name));
            }
            const auto valid_type = type_list.is_valid_type(ty) ? type_list.get_type(ty) : Type::create_error_type();
            const auto val = read_default_value(f, valid_type, file.get());
            read_single_char(f, ';');
            file->add_constant(name, valid_type, val);
        }
        else if(keyword == "package")
        {
            read_spaces(f);
            const auto package_name = read_ident(f);
            if(file->package_name.empty() == false)
            {
                f->report_error(fmt::format("tried to change package name from {} to {}", file->package_name, package_name));
            }
            if(file->structs.empty() == false)
            {
                f->report_error("cant change package name after adding structs");
            }
            read_spaces(f);
            read_single_char(f, ';');
            file->package_name = package_name;
        }
        else
        {
            f->report_error(fmt::format("Expected struct, enum, package or const. Found unknown ident {}", keyword));
        }

        // place file marker at the next non whitespace or at eof
        read_spaces(f);
    }

    if(f->errors.empty() == false)
    {
        return nullptr;
    }
    return file;
}
