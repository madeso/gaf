#!/usr/bin/env python3
# GAme Format Parser

# todo: GENERAL: structure python code better

# todo: verify that the user can specify other json documents than the default
# todo: string
# todo: array
# todo: enum
# todo: imgui api/extensions
# todo: allow place "extensions" in other files
# todo: json read/write
# todo: toml read/write
# todo: binary read/write

from enum import Enum
import enum
import os
import typing


@enum.unique
class Language(Enum):
    CPP = object()


@enum.unique
class StandardType(Enum):
    int8 = object()
    int16 = object()
    int32 = object()
    int64 = object()
    float = object()
    double = object()
    byte = object()
    INVALID = object()

    def get_cpp_type(self) -> str:
        if self == StandardType.int8:
            return 'int8_t'
        if self == StandardType.int16:
            return 'int16_t'
        if self == StandardType.int32:
            return 'int32_t'
        if self == StandardType.int64:
            return 'int64_t'
        if self == StandardType.float:
            return 'float'
        if self == StandardType.double:
            return 'double'
        if self == StandardType.byte:
            return 'char'
        return ''


class ParseError(Exception):
    def __init__(self, message):
        self.message = message


class CharFile:
    def __init__(self, f):
        import os
        self.name = os.path.abspath(f.name)
        self.data = f.read()
        self.index = 0
        self.line = 1

    def read(self) -> str:
        c = self.data[self.index]
        self.index += 1
        if c == '\n':
            self.line += 1
        return c

    def peek(self, count: int) -> typing.Optional[str]:
        if self.index + count >= len(self.data):
            return None
        return self.data[self.index + count]

    def report_error(self, error):
        raise ParseError('{fi}({ln}): {err}'.format(err=error, ln=self.line, fi=self.name))


def read_char(f: CharFile) -> str:
    return f.read()


def peek_char(f: CharFile, count: int = 0) -> typing.Optional[str]:
    return f.peek(count)


def is_space(ch: str) -> bool:
    if ch == ' ':
        return True
    if ch == '\n':
        return True
    if ch == '\r':
        return True
    if ch == '\t':
        return True
    return False


def is_ident(first: bool, ch: typing.Optional[str]) -> bool:
    if ch is None:
        return False
    if 'a' <= ch <= 'z':
        return True
    if 'A' <= ch <= 'Z':
        return True
    if ch == '_':
        return True
    if first is False:
        if '0' <= ch <= '9':
            return True
    return False


class Type:
    def __init__(self, standard_type: StandardType, name: str, pass_as_value: bool,
                 default_value: typing.Optional[str]=None):
        self.name = name
        self.standard_type = standard_type
        self.pass_as_value = pass_as_value
        self.default_value = default_value

    def get_cpp_type(self) -> str:
        if self.standard_type == StandardType.INVALID:
            return self.name
        else:
            return self.standard_type.get_cpp_type()


class TypeList:
    def __init__(self):
        self.types = dict()

    def add_type(self, t: Type):
        # todo: handle duplicate defined types
        self.types[t.name] = t

    def add_default_types(self):
        self.add_type(Type(StandardType.int8, 'int8', True, '0'))
        self.add_type(Type(StandardType.int16, 'int16', True, '0'))
        self.add_type(Type(StandardType.int32, 'int32', True, '0'))
        self.add_type(Type(StandardType.int64, 'int64', True, '0'))
        self.add_type(Type(StandardType.float, 'float', True, '0.0f'))
        self.add_type(Type(StandardType.double, 'double', True, '0.0'))
        self.add_type(Type(StandardType.byte, 'byte', True, '0'))

    def is_valid_type(self, name: str) -> bool:
        return name in self.types

    def get_type(self, name: str) -> Type:
        return self.types[name]


def get_cpp_parse_from_rapidjson_helper_int(t: StandardType, member: str, indent: str, name: str) -> str:
    return '{i}if(iter->value.IsInt64()==false) return "read value for {n} was not a integer"; \n' \
           '{i}else {{\n' \
           '{i}  auto gafv = iter->value.GetInt64();\n' \
           '{i}  if(gafv < std::numeric_limits<{t}>::min()) return "read value for {n} was to low";\n' \
           '{i}  if(gafv > std::numeric_limits<{t}>::max()) return "read value for {n} was to high";\n' \
           '{i}  c->{m} = static_cast<{t}>(gafv);\n' \
           '{i}}}\n'.format(m=member, i=indent, t=t.get_cpp_type(), n=name)


def get_cpp_parse_from_rapidjson_helper_float(member: str, indent: str, name: str) -> str:
    return '{i}if(iter->value.IsDouble()==false) return "read value for {n} was not a double"; \n' \
           '{i}c->{m} = iter->value.GetDouble();\n'.format(m=member, i=indent, n=name)


def get_cpp_parse_from_rapidjson(t: StandardType, member: str, indent: str, name: str):
    if t == StandardType.int8:
        return get_cpp_parse_from_rapidjson_helper_int(t, member, indent, name)
    if t == StandardType.int16:
        return get_cpp_parse_from_rapidjson_helper_int(t, member, indent, name)
    if t == StandardType.int32:
        return get_cpp_parse_from_rapidjson_helper_int(t, member, indent, name)
    if t == StandardType.int64:
        return '{i}if(iter->value.IsInt64()==false) return "read value for {n} was not a integer"; \n' \
               '{i}c->{m} = iter->value.GetInt64();\n'.format(m=member, i=indent, t=t, n=name)
    if t == StandardType.float:
        return get_cpp_parse_from_rapidjson_helper_float(member, indent, name)
    if t == StandardType.double:
        return get_cpp_parse_from_rapidjson_helper_float(member, indent, name)
    if t == StandardType.byte:
        return get_cpp_parse_from_rapidjson_helper_int(t, member, indent, name)
    return ''


def is_default_type(tn: str) -> bool:
    tl = TypeList()
    tl.add_default_types()
    return tl.is_valid_type(tn)


def read_white_spaces(f: CharFile):
    while is_space(peek_char(f)):
        read_char(f)


def read_spaces(f: CharFile):
    while True:
        read_white_spaces(f)
        if peek_char(f, 0) == '/' and peek_char(f, 1) == '/':
            while peek_char(f) != '\n':
                read_char(f)
        elif peek_char(f, 0) == '/' and peek_char(f, 1) == '*':
            slash = read_char(f)
            star = read_char(f)
            assert slash == '/'
            assert star == '*'
            while peek_char(f, 0) != '*' or peek_char(f, 1) != '/':
                read_char(f)
            star = read_char(f)
            slash = read_char(f)
            assert slash == '/'
            assert star == '*'
        else:
            return


def read_ident(f: CharFile) -> str:
    read_spaces(f)
    ident = ''
    first = True
    while is_ident(first, peek_char(f)):
        ident += read_char(f)
        first = False
    if len(ident) == 0:
        raise f.report_error('expecting ident but found {}'.format(peek_char(f)))
    return ident


def read_single_char(f: CharFile, ch: str):
    read_spaces(f)
    r = read_char(f)
    if r == ch:
        pass
    else:
        raise f.report_error('expecting char {c}, but found {r}'.format(c=ch, r=r))


class ArrayData:
    def __init__(self):
        self.total_size_number = None
        self.total_size_type = None
        self.current_size = None


class Member:
    def __init__(self, name: str, typename: Type):
        self.name = name
        self.typename = typename
        self.defaultvalue = typename.default_value
        self.array = None

    def __str__(self):
        if self.defaultvalue is None:
            return '{tn} {n};'.format(n=self.name, tn=self.typename.name)
        else:
            return '{tn} {n} = {dv};'.format(n=self.name, tn=self.typename.name, dv=self.defaultvalue)


def empty_member_list() -> typing.List[Member]:
    return []


class Struct:
    def __init__(self, name: str):
        self.name = name
        self.members = empty_member_list()

    def add_member(self, member: Member):
        self.members.append(member)

    def __str__(self):
        return 'struct {n} {{\n{mem}\n}}'.format(n=self.name, mem='\n'.join(['  ' + str(x) for x in self.members]))


def empty_struct_list() -> typing.List[Struct]:
    return []


class Constant:
    def __init__(self, n: str, t: Type, v: str):
        self.name = n
        self.type = t
        self.value = v


def empty_constant_list() -> typing.List[Constant]:
    return []


class File:
    def __init__(self):
        self.structs = empty_struct_list()
        self.constants = empty_constant_list()
        self.package_name = ''

    def __str__(self):
        package_name = ''
        if len(self.package_name) > 0:
            package_name = 'package {};\n'.format(self.package_name)
        return package_name + '\n'.join([str(x) for x in self.structs])

    def add_constant(self, n: str, t: Type, v: str):
        self.constants.append(Constant(n, t, v))

    def find_constant(self, name: str, ty: typing.Optional[Type]) -> typing.Optional[Constant]:
        for c in self.constants:
            if ty is None:
                if c.name == name:
                    return c
            elif c.name == name and c.type == ty:
                return c
        return None


def is_number(n: str) -> bool:
    return n in '0123456789'


def read_number(f: CharFile) -> str:
    ret = ''
    while is_number(peek_char(f)[0]):
        ret += read_char(f)
    if len(ret) == 0:
        f.report_error('Expected number, found {}'.format(peek_char(f)))
    return ret


def read_default_value_int(f: CharFile) -> str:
    return read_number(f)


def read_default_value_double(f: CharFile) -> str:
    dec = read_number(f)
    read_single_char(f, '.')
    frac = read_number(f)
    return '{d}.{f}'.format(d=dec, f=frac)


def read_default_value(f: CharFile, t: Type, fi: File) -> str:
    read_spaces(f)

    p = peek_char(f)
    if is_ident(True, p):
        ident = read_ident(f)
        c = fi.find_constant(ident, t)
        if c is None:
            f.report_error('failed to find constant named {n} with a type {t}'.format(n=ident, t=t))
            return ''
        return c.value

    if t.standard_type == StandardType.int8:
        return read_default_value_int(f)
    if t.standard_type == StandardType.int16:
        return read_default_value_int(f)
    if t.standard_type == StandardType.int32:
        return read_default_value_int(f)
    if t.standard_type == StandardType.int64:
        return read_default_value_int(f)
    if t.standard_type == StandardType.float:
        fl = read_default_value_double(f)
        read_single_char(f, 'f')
        return fl
    if t.standard_type == StandardType.double:
        return read_default_value_double(f)
    if t.standard_type == StandardType.byte:
        f.report_error('default value for byte is not yet supported')
        return ''
    return ''


def read_array(f: CharFile, fi: File) -> ArrayData:
    read_single_char(f, '[')
    read_spaces(f)
    ch = peek_char(f)

    first_number = None
    second_number = None
    first_type = None
    second_type = None

    if is_ident(True, ch):
        ident = read_ident(f)
        if is_default_type(ident):
            first_type = ident
        else:
            c = fi.find_constant(ident, None)
            if c is None:
                f.report_error('the first ident {} is neither a type nor a constant'.format(ident))
            else:
                first_number = c.value
    else:
        first_number = read_number(f)

    read_spaces(f)
    c = peek_char(f)
    if c == ',':
        read_single_char(f, ',')
        read_spaces(f)
        c = peek_char(f)
        if is_ident(True, c):
            ident = read_ident(f)
            if is_default_type(ident):
                second_type = ident
            else:
                constant = fi.find_constant(ident, None)
                if constant is None:
                    f.report_error('the second ident {} is neither a type nor a constant'.format(ident))
                else:
                    second_number = constant.value

        elif is_number(c):
            second_number = read_number(f)
        else:
            f.report_error('unexpected character in array expression: {}'.format(c))
    read_spaces(f)
    read_single_char(f, ']')

    array = ArrayData()

    if second_number is None and second_type is None:
        array.total_size_number = first_number
        array.total_size_type = first_type
    elif first_type is not None:
        array.current_size = first_type
        array.total_size_number = second_number
        array.total_size_type = second_type
    else:
        if first_number is not None:
            f.report_error('Current size is always {}'.format(first_number))
        else:
            f.report_error('Invalid state')

    return array


def read_struct(f: CharFile, type_list: TypeList, fi: File) -> Struct:
    struct_name = read_ident(f)
    struct = Struct(struct_name)
    read_single_char(f, '{')
    while peek_char(f) != '}':
        ty = read_ident(f)
        name = read_ident(f)
        read_spaces(f)
        ch = peek_char(f)
        if type_list.is_valid_type(ty) is False:
            f.report_error('Invalid type {t} for member {s}.{m}'.format(t=ty, s=struct_name, m=name))
        valid_type = type_list.get_type(ty) if type_list.is_valid_type(ty) else StandardType.int32
        mem = Member(name, valid_type)

        if ch == '[':
            array = read_array(f, fi)

            mem.array = array

            read_spaces(f)
            ch = peek_char(f)

        if ch == '=':
            if not is_default_type(ty):
                f.report_error('structs cant have default values yet')
            read_char(f)
            mem.defaultvalue = read_default_value(f, valid_type, fi)
        read_spaces(f)
        read_single_char(f, ';')

        struct.add_member(mem)
        read_spaces(f)
    read_single_char(f, '}')
    type_list.add_type(Type(StandardType.INVALID, struct_name, False))

    return struct


def read_several_structs(f: CharFile) -> File:
    file = File()
    read_spaces(f)
    type_list = TypeList()
    type_list.add_default_types()
    while peek_char(f) is not None:
        keyword = read_ident(f)
        if keyword == 'struct':
            s = read_struct(f, type_list, file)
            file.structs.append(s)
        elif keyword == 'const':
            ty = read_ident(f)
            name = read_ident(f)
            read_spaces(f)
            read_single_char(f, '=')
            if type_list.is_valid_type(ty) is False:
                f.report_error('Invalid type {t} for const {m}'.format(t=ty, m=name))
            valid_type = type_list.get_type(ty) if type_list.is_valid_type(ty) else StandardType.int32
            val = read_default_value(f, valid_type, file)
            read_single_char(f, ';')
            file.add_constant(name, valid_type, val)
        elif keyword == 'package':
            read_spaces(f)
            package_name = read_ident(f)
            if len(file.package_name) > 0:
                f.report_error(
                    'tried to change package name from {old} to {new}'.format(old=file.package_name, new=package_name))
            if len(file.structs) > 0:
                f.report_error('cant change package name after adding structs')
            read_spaces(f)
            read_single_char(f, ';')
            file.package_name = package_name
        else:
            raise f.report_error('Expected struct, package or const. Found unknown ident {}'.format(keyword))
        read_spaces(f)  # place file marker at the next non whitespace or at eof

    return file


def merge(iters):
    for it in iters:
        yield from it


class Out:
    def __init__(self):
        self.header = []
        self.source = []

    def add_source(self, line: str):
        self.source.append(line)

    def add_header(self, line: str):
        self.header.append(line)


def write_json_source_for_cpp(write_json: bool, sources: Out, s: Struct):
    if write_json:
        sources.add_source('const char* ReadFromJsonValue({}* c, const rapidjson::Value& value) {{\n'.format(s.name))
        sources.add_source('  if(!value.IsObject()) return "tried to read {} but value was not a object";\n'.format(s.name))
        sources.add_source('  rapidjson::Value::ConstMemberIterator iter;\n')
        for m in s.members:
            sources.add_source('  iter = value.FindMember("{n}");\n'.format(n=m.name))
            sources.add_source('  if(iter != value.MemberEnd()) {\n')
            if m.typename.standard_type != StandardType.INVALID:
                sources.add_source(get_cpp_parse_from_rapidjson(m.typename.standard_type, m.name, '    ', m.name))
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


def get_unique_types(f: File) -> typing.Set[Type]:
    return set(m.typename for m in merge(s.members for s in f.structs))


def generate_cpp(f: File, sources: Out, name: str, header_only: bool, write_json: bool):
    headerguard = 'GENERATED_' + name.upper()

    # get all standard types used for typedefing later on...
    unique_types = get_unique_types(f)
    default_types = [t for t in unique_types
                     if t.standard_type.get_cpp_type() != '' and t.name != t.get_cpp_type()
                     ]

    sources.add_header('#ifndef {}_H\n'.format(headerguard))
    sources.add_header('#define {}_H\n'.format(headerguard))
    sources.add_header('\n')
    if len(default_types) > 0:
        sources.add_header('\n')
        sources.add_header('#include <cstdint>\n')
        sources.add_header('\n')

    if write_json and header_only:
        sources.add_header('#include <limits>\n')
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

        if header_only and write_json:
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


def on_generate_command(args):
    file = CharFile(args.input)
    if args.debug:
        s = read_several_structs(file)
    else:
        try:
            s = read_several_structs(file)
        except ParseError as p:
            print(p.message)
            return
    if args.language == Language.CPP:
        write_cpp(s, args, args.output_folder,
                  os.path.splitext(os.path.basename(file.name))[0] if args.name is None else args.name)
    else:
        raise ParseError('unhandled language {}'.format(args.language))


def on_display_command(args):
    if args.debug:
        s = read_several_structs(CharFile(args.file))
    else:
        try:
            s = read_several_structs(CharFile(args.file))
        except ParseError as p:
            print(p.message)
            return
    print(s)


def main():
    import argparse

    class EnumType(object):
        """Factory for creating enum object types
        """

        def __init__(self, enumclass):
            self.enums = enumclass

        def __call__(self, astring):
            name = self.enums.__name__
            try:
                return self.enums[astring.upper()]
            except KeyError:
                msg = ', '.join([t.name.lower() for t in self.enums])
                msg = '%s: use one of {%s}' % (name, msg)
                raise argparse.ArgumentTypeError(msg)

        def __repr__(self):
            astr = ', '.join([t.name.lower() for t in self.enums])
            return '%s(%s)' % (self.enums.__name__, astr)

    parser = argparse.ArgumentParser(description='GAme Format parser', fromfile_prefix_chars='@')
    parser.set_defaults(func=None)
    sub = parser.add_subparsers(help='sub-command help')

    gen_parser = sub.add_parser('generate', help='generate a game format parser', aliases=['gen'],
                                fromfile_prefix_chars='@')
    gen_parser.add_argument('language', type=EnumType(Language), help='the language')
    gen_parser.add_argument('input', type=argparse.FileType('r'), help='the source gaf file')
    gen_parser.add_argument('output_folder', help='the output directory')
    gen_parser.add_argument('--debug', action='store_const', const=True, default=False, help='debug gaf')
    gen_parser.add_argument('--name', help='use this name instead of the autogenerated name')
    gen_parser.add_argument('--header-only', action='store_const', const=True, default=False,
                            help='header only implementation')
    gen_parser.add_argument('--include-json', action='store_const', const=True, default=False,
                            help='include rapid json implementation')
    gen_parser.set_defaults(func=on_generate_command)

    dis_parser = sub.add_parser('display', help='generate a game format parser', aliases=['disp', 'print', 'prn'])
    dis_parser.add_argument('file', type=argparse.FileType('r'), help='the source gaf file')
    dis_parser.add_argument('--debug', action='store_const', const=False, default=True, help='debug gaf')
    dis_parser.set_defaults(func=on_display_command)

    args = parser.parse_args()
    if args.func is not None:
        args.func(args)


if __name__ == "__main__":
    main()
