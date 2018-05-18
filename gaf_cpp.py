#!/usr/bin/env python3
import os

from gaf_types import StandardType, Struct, get_unique_types, File


class Out:
    def __init__(self):
        self.header = []
        self.source = []

    def add_source(self, line: str):
        self.source.append(line)

    def add_header(self, line: str):
        self.header.append(line)


def get_cpp_parse_from_rapidjson_helper_int(sources: Out, t: StandardType, member: str, indent: str, name: str) -> None:
    line = '{i}if(iter->value.IsInt64()==false) return "read value for {n} was not a integer"; \n' \
           '{i}else {{\n' \
           '{i}  auto gafv = iter->value.GetInt64();\n' \
           '{i}  if(gafv < std::numeric_limits<{t}>::min()) return "read value for {n} was to low";\n' \
           '{i}  if(gafv > std::numeric_limits<{t}>::max()) return "read value for {n} was to high";\n' \
           '{i}  c->{m} = static_cast<{t}>(gafv);\n' \
           '{i}}}\n'.format(m=member, i=indent, t=t.get_cpp_type(), n=name)
    sources.add_source(line)


def get_cpp_parse_from_rapidjson_helper_float(sources: Out, member: str, indent: str, name: str) -> None:
    line = '{i}if(iter->value.IsDouble()==false) return "read value for {n} was not a double"; \n' \
           '{i}c->{m} = iter->value.GetDouble();\n'.format(m=member, i=indent, n=name)
    sources.add_source(line)


def get_cpp_parse_from_rapidjson(sources: Out, t: StandardType, member: str, indent: str, name: str) -> None:
    if t == StandardType.int8:
        get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name)
    elif t == StandardType.int16:
        get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name)
    elif t == StandardType.int32:
        get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name)
    elif t == StandardType.int64:
        line = '{i}if(iter->value.IsInt64()==false) return "read value for {n} was not a integer"; \n' \
               '{i}c->{m} = iter->value.GetInt64();\n'.format(m=member, i=indent, t=t, n=name)
        sources.add_source(line)
    elif t == StandardType.float:
        get_cpp_parse_from_rapidjson_helper_float(sources, member, indent, name)
    elif t == StandardType.double:
        get_cpp_parse_from_rapidjson_helper_float(sources, member, indent, name)
    elif t == StandardType.byte:
        get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name)
    elif t == StandardType.string:
        line = '{i}if(iter->value.IsString()==false) return "read value for {n} was not a string"; \n' \
               '{i}c->{m} = iter->value.GetString();\n'.format(m=member, i=indent, t=t, n=name)
        sources.add_source(line)
    else:
        print('BUG: No type specified')


def write_json_source_for_cpp(write_json: bool, sources: Out, s: Struct):
    if write_json:
        sources.add_source('const char* ReadFromJsonValue({}* c, const rapidjson::Value& value) {{\n'.format(s.name))
        sources.add_source('  if(!value.IsObject()) return "tried to read {} but value was not a object";\n'.format(s.name))
        sources.add_source('  rapidjson::Value::ConstMemberIterator iter;\n')
        for m in s.members:
            sources.add_source('  iter = value.FindMember("{n}");\n'.format(n=m.name))
            sources.add_source('  if(iter != value.MemberEnd()) {\n')
            if m.typename.standard_type != StandardType.INVALID:
                get_cpp_parse_from_rapidjson(sources, m.typename.standard_type, m.name, '    ', m.name)
            else:
                sources.add_source(
                    '   {{  const char* const r = ReadFromJsonValue(&c->{},iter->value); if(r!=nullptr) {{ return r; }} }}\n'
                    .format(m.name))
            sources.add_source('  }\n')
            sources.add_source('  else {\n')
            sources.add_source('    return "missing {} in json object";\n'.format(m.name))
            sources.add_source('  }\n')
        sources.add_source('  return nullptr;\n')
        sources.add_source('}\n')
        sources.add_source('\n')


def write_member_variables_for_cpp(sources: Out, s: Struct):
    # sources.add_header(' public:\n')
    for m in s.members:
        sources.add_header('  {tn} {n};\n'.format(n=m.name, tn=m.typename.name))
    sources.add_header('}}; // class {}\n'.format(s.name))


def write_default_constructor_for_cpp(s: Struct, sources: Out):
    common_members = [x for x in s.members if x.defaultvalue is not None]
    if len(common_members)>0:
        sources.add_header('  {}();\n'.format(s.name))
        sources.add_header('\n')
        sources.add_source('{n}::{n}()\n'.format(n=s.name))
        sep = ':'
        for m in common_members:
            sources.add_source('  {s} {n}({d})\n'.format(s=sep, n=m.name, d=m.defaultvalue))
            sep = ','
        sources.add_source('{}\n')
        sources.add_source('\n')


def generate_cpp(f: File, sources: Out, name: str, header_only: bool, write_json: bool):
    headerguard = 'GENERATED_' + name.upper()

    # get all standard types used for typedefing later on...
    unique_types = get_unique_types(f)
    default_types = [t for t in unique_types
                     if t.standard_type.get_cpp_type() != '' and t.name != t.get_cpp_type()
                     ]

    has_string = StandardType.string in [t.standard_type for t in unique_types]

    sources.add_header('#ifndef {}_H\n'.format(headerguard))
    sources.add_header('#define {}_H\n'.format(headerguard))
    sources.add_header('\n')
    if len(default_types) > 0:
        sources.add_header('\n')
        sources.add_header('#include <cstdint>\n')
        if has_string:
            sources.add_header('#include <string>\n')
        sources.add_header('\n')

    if write_json and header_only:
        sources.add_header('#include <limits>\n')
        sources.add_header('#include "rapidjson/document.h"\n')
        sources.add_header('\n')

    if write_json and not header_only:
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

    for s in f.structs:
        sources.add_header('class {} {{\n'.format(s.name))
        sources.add_header(' public:\n')
        write_default_constructor_for_cpp(s, sources)

        write_json_source_for_cpp(write_json, sources, s)
        write_member_variables_for_cpp(sources, s)

        if write_json:
            sources.add_header('\n')
            sources.add_header('const char* ReadFromJsonValue({}* c, const rapidjson::Value& value);\n'.format(s.name))
        sources.add_header('\n')

    if header_only:
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


def write_cpp(f: File, args, out_dir: str, name: str):
    header_only = args.header_only
    write_json = args.include_json

    sources = Out()
    generate_cpp(f, sources, name, header_only, write_json)

    with open(os.path.join(out_dir, name + '.h'), 'w', encoding='utf-8') as out:
        for s in sources.header:
            out.write(s)

    if not header_only:
        with open(os.path.join(out_dir, name + '.cc'), 'w', encoding='utf-8') as out:
            out.write('#include "{}.h"\n'.format(name))
            out.write('\n')
            out.write('#include <limits>\n')
            if write_json:
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
