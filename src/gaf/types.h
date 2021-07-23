#pragma once

#include <string_view>
#include <string>
#include <optional>
#include <vector>
#include <map>
#include <memory>
#include <set>

struct Args;

enum class StandardType
{
    Int8,
    Int16,
    Int32,
    Int64,
    Uint8,
    Uint16,
    Uint32,
    Uint64,
    Float,
    Double,
    Byte,
    Bool,
    String,
    INVALID,
};

constexpr std::string_view get_cpp_type(StandardType type)
{
    switch(type)
    {
        case StandardType::Int8: return "std::int8_t";
        case StandardType::Int16: return "std::int16_t";
        case StandardType::Int32: return "std::int32_t";
        case StandardType::Int64: return "std::int64_t";
        case StandardType::Uint8: return "std::uint8_t";
        case StandardType::Uint16: return "std::uint16_t";
        case StandardType::Uint32: return "std::uint32_t";
        case StandardType::Uint64: return "std::uint64_t";
        case StandardType::Float: return "float";
        case StandardType::Double: return "double";
        case StandardType::Byte: return "char";
        case StandardType::Bool: return "bool";
        case StandardType::String: return "std::string";
        default: return "";
    }
}

struct Type
{
    StandardType standard_type;
    std::string name;
    bool is_int;
    std::optional<std::string> default_value;
    bool is_enum;

    Type
    (
        StandardType s,
        std::string n,
        bool i,
        std::optional<std::string> d = {},
        bool e = false
    );
    
    std::string get_cpp_type() const;

    static Type create_error_type();
};

struct TypeList
{
    std::map<std::string, Type> types;

    void add_type(const Type& t);
    void add_default_types();
    bool is_valid_type(const std::string& name);
    Type get_type(const std::string& name) const;
};

struct Member
{
    std::string name;
    Type type_name;
    std::optional<std::string> defaultvalue;
    bool is_dynamic_array;
    bool is_optional;
    bool missing_is_fail;

    Member(const std::string& n, const Type& t);
};

struct Struct
{
    std::string name;
    std::vector<Member> members;
    bool is_defined = false;

    explicit Struct(const std::string& n);
};

struct Enum
{
    std::string name;
    std::vector<std::string> values;
    std::set<std::string> sorted_values;
    std::optional<Type> type;

    explicit Enum(const std::string& n);

    bool is_value(const std::string& v) const;
    void add_value(const std::string& v);
};


struct Constant
{
    std::string name;
    Type type;
    std::string value;

    Constant
    (
        const std::string& n,
        const Type& t,
        const std::string& v
    );
};



struct File
{
    std::vector<std::shared_ptr<Struct>> structs;
    std::vector<std::shared_ptr<Struct>> typedefs;
    std::vector<std::shared_ptr<Struct>> structs_defined;
    std::vector<std::shared_ptr<Enum>> enums;
    std::vector<std::shared_ptr<Constant>> constants;
    std::string package_name;

    void add_constant(const std::string& n, const Type& t, const std::string& v);
    std::shared_ptr<Constant> find_constant(const std::string& name, std::optional<Type> ty) const;
    std::shared_ptr<Enum> find_enum(const std::string& name) const;
    std::shared_ptr<Struct> find_struct(const std::string& name) const;
};

bool is_default_type(const std::string& tn);

struct Args;

struct FileOut
{
    virtual ~FileOut() = default;
    virtual void write(const std::string& line) = 0;
};

struct PrettyFileOut : FileOut
{
    std::unique_ptr<FileOut> dest;
    int indent;
    
    explicit PrettyFileOut(std::unique_ptr<FileOut>&& d);
    void write(const std::string& line) override;
};

struct Writer
{
    virtual ~Writer() = default;
    virtual std::unique_ptr<FileOut> open(const std::string& f) = 0;
};

struct Plugin
{
    virtual ~Plugin() = default;

    virtual std::string get_name() = 0;
    virtual int run_plugin(const File& file, Writer* writer, std::string& output_folder, Args& args, const std::string& name) = 0;
};

std::set<std::string> get_headers_types(const File& f);


std::ostream& operator<<(std::ostream& s, const File& f);
