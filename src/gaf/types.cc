#include "gaf/types.h"

#include <algorithm>
#include <cassert>
#include <ostream>
#include <set>

Type::Type(StandardType s, std::string n, bool i, std::optional<std::string> d, bool e)
    : standard_type(s)
    , name(n)
    , is_int(i)
    , default_value(d)
    , is_enum(e)
{
}

std::string Type::get_cpp_type() const
{
    if (standard_type == StandardType::INVALID)
    {
        return name;
    }
    else
    {
        const auto str = ::get_cpp_type(standard_type);
        return {str.begin(), str.end()};
    }
}

Type Type::create_error_type()
{
    return {StandardType::Int32, "int32", true, "0"};
}

bool operator==(const Type& lhs, const Type& rhs)
{
    return lhs.standard_type == rhs.standard_type && lhs.name == rhs.name && lhs.is_int == rhs.is_int &&
           lhs.default_value == rhs.default_value && lhs.is_enum == rhs.is_enum;
}

void TypeList::add_type(const Type& t)
{
    // todo: handle duplicate defined types
    types.emplace(t.name, t);
}

void TypeList::add_default_types()
{
    add_type({StandardType::Int8, "int8", true, "0"});
    add_type({StandardType::Int16, "int16", true, "0"});
    add_type({StandardType::Int32, "int32", true, "0"});
    add_type({StandardType::Int64, "int64", true, "0"});
    add_type({StandardType::Uint8, "uint8", true, "0"});
    add_type({StandardType::Uint16, "uint16", true, "0"});
    add_type({StandardType::Uint32, "uint32", true, "0"});
    add_type({StandardType::Uint64, "uint64", true, "0"});

    add_type({StandardType::Float, "float", false, "0.0"});
    add_type({StandardType::Double, "double", false, "0.0"});
    add_type({StandardType::Bool, "bool", false, "false"});
    add_type({StandardType::String, "string", false});
}

bool TypeList::is_valid_type(const std::string& name)
{
    return types.find(name) != types.end();
}

Type TypeList::get_type(const std::string& name) const
{
    const auto found = types.find(name);
    assert(found != types.end());
    return found->second;
}

bool is_default_type(const std::string& tn)
{
    auto tl = TypeList{};
    tl.add_default_types();
    return tl.is_valid_type(tn);
}

Member::Member(const std::string& n, const Type& t)
    : name(n)
    , type_name(t)
    , defaultvalue(t.default_value)
    , is_dynamic_array(false)
    , is_optional(false)
    , missing_is_fail(true)
{
}

std::ostream& operator<<(std::ostream& s, const Member& self)
{
    if (self.defaultvalue.has_value() == false)
        s << self.type_name.name << " " << self.name << ";";
    else
        s << self.type_name.name << " " << self.name << " = " << *self.defaultvalue << ";";
    return s;
}

std::ostream& operator<<(std::ostream& s, const Struct& self)
{
    s << "struct " << self.name << "\n{\n";

    for (const auto& m : self.members)
    {
        s << "    " << m << "\n";
    }
    s << "}";

    return s;
}

Struct::Struct(const std::string& n)
    : name(n)
{
}

Enum::Enum(const std::string& n)
    : name(n)
{
}

bool Enum::is_value(const std::string& v) const
{
    return sorted_values.find(v) != sorted_values.end();
}

void Enum::add_value(const std::string& v)
{
    values.emplace_back(v);
    const auto inserted = sorted_values.emplace(v).second;
    assert(inserted);
}

Constant::Constant(const std::string& n, const Type& t, const std::string& v)
    : name(n)
    , type(t)
    , value(v)
{
}

void File::add_constant(const std::string& n, const Type& t, const std::string& v)
{
    constants.emplace_back(std::make_shared<Constant>(n, t, v));
}

std::shared_ptr<Constant> File::find_constant(const std::string& name, std::optional<Type> ty) const
{
    for (const auto& c : constants)
    {
        if (ty.has_value() == false)
        {
            if (c->name == name)
            {
                return c;
            }
        }
        else if (c->name == name && c->type == *ty)
        {
            return c;
        }
    }
    return nullptr;
}

std::shared_ptr<Enum> File::find_enum(const std::string& name) const
{
    for (const auto& e : enums)
    {
        if (e->name == name)
        {
            return e;
        }
    }
    return nullptr;
}

std::shared_ptr<Struct> File::find_struct(const std::string& name) const
{
    auto found = named_structs.find(name);
    if (found == named_structs.end())
    {
        return nullptr;
    }
    else
    {
        return found->second;
    }
}

std::ostream& operator<<(std::ostream& s, const File& f)
{
    if (f.package_name.empty() == false)
    {
        s << "package " << f.package_name << ";\n";
    }
    for (const auto& x : f.structs)
    {
        s << *x << '\n';
    }
    return s;
}

PrettyFileOut::PrettyFileOut(std::unique_ptr<FileOut>&& d)
    : dest(std::move(d))
    , indent(0)
{
}

void PrettyFileOut::write(const std::string& line)
{
    const int dec = static_cast<int>(std::count(line.begin(), line.end(), '}'));
    const int inc = static_cast<int>(std::count(line.begin(), line.end(), '{'));
    indent -= dec;
    if (dec > 0)
    {
        indent += inc;
    }
    assert(indent >= 0);
    const auto current = [this, &line]() -> int
    {
        if (line.empty())
        {
            return 0;
        }
        switch (line[0])
        {
        case ':':
        case ',':
            return indent + 1;
        default:
            return indent;
        }
    }();
    dest->write(std::string(current * 4, ' ') + line + "\n");
    if (dec <= 0)
    {
        indent += inc;
    }
}
