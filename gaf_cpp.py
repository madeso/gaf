#!/usr/bin/env python3
import os

from gaf_types import StandardType, Struct, get_unique_types, File, Member, OutputOptions, CppEnumStyle, Enum


class Out:
    def __init__(self):
        self.header = []
        self.source = []

    def add_source(self, line: str):
        self.source.append(line)

    def add_header(self, line: str):
        self.header.append(line)


class VarValue:
    def __init__(self, variable: str, value: str):
        self.variable = variable
        self.value = value


def get_cpp_parse_from_rapidjson_helper_int(sources: Out, t: StandardType, member: str, indent: str, name: str, json: str) -> VarValue:
    line = '{i}if({j}.IsInt64()==false) return "read value for {n} was not a integer"; \n' \
           '{i}auto gafv = {j}.GetInt64();\n' \
           '{i}if(gafv < std::numeric_limits<{t}>::min()) return "read value for {n} was to low";\n' \
           '{i}if(gafv > std::numeric_limits<{t}>::max()) return "read value for {n} was to high";\n'\
        .format(m=member, i=indent, t=t.get_cpp_type(), n=name, j=json)
    sources.add_source(line)
    var = 'c->{m}'.format(m=member)
    val = 'static_cast<{t}>(gafv)'.format(t=t.get_cpp_type())
    return VarValue(variable=var, value=val)


def get_cpp_parse_from_rapidjson_helper_float(sources: Out, member: str, indent: str, name: str, json: str) -> VarValue:
    line = '{i}if({j}.IsDouble()==false) return "read value for {n} was not a double"; \n'\
        .format(m=member, i=indent, n=name, j=json)
    sources.add_source(line)
    var = 'c->{m}'.format(m=member)
    val = '{}.GetDouble()'.format(json)
    return VarValue(variable=var, value=val)


def get_cpp_parse_from_rapidjson_base(sources: Out, t: StandardType, member: str, indent: str, name: str, json: str) -> VarValue:
    if t == StandardType.int8:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.int16:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.int32:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.int64:
        line = '{i}if({j}.IsInt64()==false) return "read value for {n} was not a integer"; \n'\
            .format(m=member, i=indent, t=t, n=name, j=json)
        var = 'c->{m}'.format(m=member)
        val = '{}.GetInt64()'.format(json)
        sources.add_source(line)
        return VarValue(variable=var, value=val)
    elif t == StandardType.float:
        return get_cpp_parse_from_rapidjson_helper_float(sources, member, indent, name, json)
    elif t == StandardType.double:
        return get_cpp_parse_from_rapidjson_helper_float(sources, member, indent, name, json)
    elif t == StandardType.byte:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.bool:
        line = '{i}if({j}.IsBool()==false) return "read value for {n} was not a bool"; \n'\
            .format(m=member, i=indent, t=t, n=name, j=json)
        var = 'c->{m}'.format(m=member)
        val = '{}.GetBool()'.format(json)
        sources.add_source(line)
        return VarValue(variable=var, value=val)
    elif t == StandardType.string:
        line = '{i}if({j}.IsString()==false) return "read value for {n} was not a string"; \n'\
            .format(m=member, i=indent, t=t, n=name, j=json)
        var = 'c->{m}'.format(m=member)
        val = '{}.GetString()'.format(json)
        sources.add_source(line)
        return VarValue(variable=var, value=val)
    else:
        print('BUG: No type specified')
        return VarValue(variable=member, value='bug_unhandled_std_type')


def get_cpp_parse_from_rapidjson(sources: Out, t: StandardType, member: str, indent: str, name: str, member_type: Member) -> None:
    if member_type.is_dynamic_array:
        line = '{i}const rapidjson::Value& arr = iter->value;\n' \
               '{i}if(!arr.IsArray()) return "tried to read {name} but value was not a array";\n' \
               '{i}for (rapidjson::SizeType i=0; i<arr.Size(); i++)\n' \
               '{i}{{\n' \
            .format(i=indent, name=name)
        sources.add_source(line)
        vv = get_cpp_parse_from_rapidjson_base(sources, t, member, indent + '  ', name, 'arr[i]')
        sources.add_source('{i}  {var}.push_back({val});\n'.format(i=indent, var=vv.variable, val=vv.value))
        sources.add_source('{}}}\n'.format(indent))
    else:
        vv = get_cpp_parse_from_rapidjson_base(sources, t, member, indent, name, 'iter->value')
        sources.add_source('{i}{var} = {val};\n'.format(i=indent, var=vv.variable, val=vv.value))


def write_json_member(m: Member, sources: Out, indent):
    if m.typename.standard_type != StandardType.INVALID:
        get_cpp_parse_from_rapidjson(sources, m.typename.standard_type, m.name, indent, m.name, m)
    else:
        if m.is_dynamic_array:
            line = '{i}const rapidjson::Value& arr = iter->value;\n' \
                   '{i}if(!arr.IsArray()) return "tried to read {name} but value was not a array";\n' \
                   '{i}for (rapidjson::SizeType i=0; i<arr.Size(); i++)\n' \
                   '{i}{{\n' \
                   '{i}  {type} temp;\n' \
                   '{i}  const char* const r = ReadFromJsonValue(&temp,arr[i]);\n' \
                   '{i}  if(r!=nullptr) {{ return r; }}\n' \
                   '{i}  c->{name}.push_back(temp);\n' \
                   '{i}}}\n'\
                .format(i=indent, name=m.name, type=m.typename.name)
            sources.add_source(line)
        else:
            sources.add_source('{i}const char* const r = ReadFromJsonValue(&c->{name},iter->value);\n'
                               '{i}if(r!=nullptr)\n'
                               '{i}{{\n'
                               '{i}  return r;\n'
                               '{i}}}\n'
                               .format(i=indent, name=m.name))


def write_json_source_for_cpp(write_json: bool, sources: Out, s: Struct):
    if write_json:
        sources.add_source('const char* ReadFromJsonValue({}* c, const rapidjson::Value& value) {{\n'.format(s.name))
        sources.add_source('  if(!value.IsObject()) return "tried to read {} but value was not a object";\n'.format(s.name))
        sources.add_source('  rapidjson::Value::ConstMemberIterator iter;\n')
        for m in s.members:
            sources.add_source('  iter = value.FindMember("{n}");\n'.format(n=m.name))
            sources.add_source('  if(iter != value.MemberEnd()) {\n')
            write_json_member(m, sources, '    ')
            sources.add_source('  }\n')
            sources.add_source('  else {\n')
            sources.add_source('    return "missing {} in json object";\n'.format(m.name))
            sources.add_source('  }\n')
        sources.add_source('  return nullptr;\n')
        sources.add_source('}\n')
        sources.add_source('\n')


def write_member_variables_for_cpp(sources: Out, s: Struct, opt: OutputOptions):
    # sources.add_header(' public:\n')
    for m in s.members:

        # m.typename.is_enum
        type_name = '{}::Type'.format(m.typename.name)\
            if m.typename.is_enum and opt.enum_style == CppEnumStyle.NamespaceEnum else m.typename.name
        if m.is_dynamic_array:
            sources.add_header('  std::vector<{tn}> {n};\n'.format(n=m.name, tn=type_name))
        else:
            sources.add_header('  {tn} {n};\n'.format(n=m.name, tn=type_name))
    sources.add_header('}}; // class {}\n'.format(s.name))


def write_default_constructor_for_cpp(s: Struct, sources: Out, opt: OutputOptions):
    common_members = [x for x in s.members if x.defaultvalue is not None]
    if len(common_members) > 0:
        sources.add_header('  {}();\n'.format(s.name))
        sources.add_header('\n')
        sources.add_source('{n}::{n}()\n'.format(n=s.name))
        sep = ':'
        for m in common_members:
            default_value = m.defaultvalue
            if m.typename.is_enum:
                default_value = '{t}{colon}{v}'.format(t=m.typename.name, v=m.defaultvalue,
                                                       colon='_' if opt.enum_style == CppEnumStyle.PrefixEnum else '::')
            sources.add_source('  {s} {n}({d})\n'.format(s=sep, n=m.name, d=default_value))
            sep = ','
        sources.add_source('{}\n')
        sources.add_source('\n')


def iterate_enum(e: Enum, sources: Out, prefix_prop: bool=False):
    prefix = '{}_'.format(e.name) if prefix_prop else ''
    for i, v in enumerate(e.values, start=1):
        last = i == len(e.values)
        comma = '' if last else ','
        sources.add_header('  {p}{v}{c}\n'.format(p=prefix, v=v, c=comma))


def add_enum_json_function(e: Enum, sources: Out, prefix_prop: bool=False, type_enum: bool=False):
    enum_type = '{}::Type'.format(e.name) if type_enum else e.name
    value_prefix = '{}_'.format(e.name) if prefix_prop else '{}::'.format(e.name)
    sources.add_header('const char* ReadFromJsonValue({t}* c, const rapidjson::Value& value);\n'.format(t=enum_type))
    sources.add_source('const char* ReadFromJsonValue({t}* c, const rapidjson::Value& value)\n'.format(t=enum_type))
    sources.add_source('{{\n')
    sources.add_source('  if(value.IsString()==false) return "read value for {e} was not a string";\n'.format(e=e.name))
    for v in e.values:
        sources.add_source('  if(strcmp(value.GetString(), "{v}")==0) {{ *c = {p}{v}; return nullptr;}}\n'.format(p=value_prefix, v=v))
    sources.add_source('  return "read string for {e} was not valud";\n'.format(e=e.name))
    sources.add_source('}}\n')
    sources.add_source('\n')


def generate_cpp(f: File, sources: Out, name: str, opt: OutputOptions):
    headerguard = 'GENERATED_' + name.upper()

    # get all standard types used for typedefing later on...
    unique_types = get_unique_types(f)
    default_types = [t for t in unique_types
                     if t.standard_type.get_cpp_type() != '' and t.name != t.get_cpp_type()
                     ]

    has_string = StandardType.string in [t.standard_type for t in unique_types]
    has_dynamic_arrays = any(m for s in f.structs for m in s.members if m.is_dynamic_array)

    sources.add_header('#ifndef {}_H\n'.format(headerguard))
    sources.add_header('#define {}_H\n'.format(headerguard))
    sources.add_header('\n')

    added_include = False
    if len(default_types) > 0:
        added_include = True
        sources.add_header('#include <cstdint>\n')
    if has_string:
        added_include = True
        sources.add_header('#include <string>\n')
    if len(f.enums) > 0 and opt.write_json:
        if opt.header_only:
            sources.add_header('#include <cstring>\n')
        else:
            sources.add_source('#include <cstring>\n')
    if has_dynamic_arrays:
        added_include = True
        sources.add_header('#include <vector>\n')
    if added_include:
        sources.add_header('\n')

    if opt.write_json and opt.header_only:
        sources.add_header('#include <limits>\n')
        sources.add_header('#include "rapidjson/document.h"\n')
        sources.add_header('\n')

    if opt.write_json and not opt.header_only:
        # todo: forward decalre rapidjson::Value
        sources.add_header('#include "rapidjson/document.h"\n')
        sources.add_header('\n')

    if f.package_name != '':
        sources.add_header('namespace {} {{\n'.format(f.package_name))
        sources.add_header('\n')

    if len(default_types) > 0:
        sources.add_header('\n')
        for t in default_types:
            sources.add_header('typedef {ct} {t};\n'.format(t=t.name, ct=t.get_cpp_type()))
        sources.add_header('\n')

    for e in f.enums:
        if opt.enum_style == CppEnumStyle.EnumClass:
            sources.add_header('enum class {} {{\n'.format(e.name))
            iterate_enum(e, sources)
            sources.add_header('}}; // enum {}\n'.format(e.name))
            if opt.write_json:
                add_enum_json_function(e, sources)
        elif opt.enum_style == CppEnumStyle.NamespaceEnum:
            sources.add_header('namespace {} {{ enum Type {{\n'.format(e.name))
            iterate_enum(e, sources)
            sources.add_header('}}; }} // namespace enum {}\n'.format(e.name))
            if opt.write_json:
                add_enum_json_function(e, sources, type_enum=True)
        elif opt.enum_style == CppEnumStyle.PrefixEnum:
            sources.add_header('enum {} {{\n'.format(e.name))
            iterate_enum(e, sources, True)
            sources.add_header('}}; // enum {}\n'.format(e.name))
            if opt.write_json:
                add_enum_json_function(e, sources, prefix_prop=True)
        else:
            sources.add_header('code generation failed, unhandled enum style {}'.format(opt.enum_style))

        sources.add_header('\n')

    for s in f.structs:
        sources.add_header('class {} {{\n'.format(s.name))
        sources.add_header(' public:\n')
        write_default_constructor_for_cpp(s, sources, opt)

        write_json_source_for_cpp(opt.write_json, sources, s)
        write_member_variables_for_cpp(sources, s, opt)

        if opt.write_json:
            sources.add_header('\n')
            sources.add_header('const char* ReadFromJsonValue({}* c, const rapidjson::Value& value);\n'.format(s.name))
        sources.add_header('\n')

    if opt.header_only:
        sources.add_header('#ifdef {}_IMPLEMENTATION\n'.format(headerguard))
        sources.add_header('\n')
        for s in sources.source:
            sources.add_header(s)
        sources.add_header('#endif // {}_IMPLEMENTATION\n'.format(headerguard))

    if f.package_name != '':
        sources.add_header('}} // namespace {}\n'.format(f.package_name))
        sources.add_header('\n')
    sources.add_header('\n')
    sources.add_header('#endif  // {}_H\n'.format(headerguard))


def write_cpp(f: File, opt: OutputOptions, out_dir: str, name: str):
    sources = Out()
    generate_cpp(f, sources, name, opt)

    with open(os.path.join(out_dir, name + '.h'), 'w', encoding='utf-8') as out:
        for s in sources.header:
            out.write(s)

    if not opt.header_only:
        with open(os.path.join(out_dir, name + '.cc'), 'w', encoding='utf-8') as out:
            out.write('#include "{}.h"\n'.format(name))
            out.write('\n')
            out.write('#include <limits>\n')
            if opt.write_json:
                out.write('#include "rapidjson/document.h"\n')
            out.write('\n')

            if f.package_name != '':
                out.write('namespace {} {{\n'.format(f.package_name))
                out.write('\n')

            for s in sources.source:
                out.write(s)

            if f.package_name != '':
                out.write('}} // namespace {}\n'.format(f.package_name))
                out.write('\n')
