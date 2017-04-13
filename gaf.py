#!/usr/bin/env python3
# GAme Format Parser

from enum import Enum
import enum
import os

@enum.unique
class Language(Enum):
    CPP = object()


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

    def read(self):
        c = self.data[self.index]
        self.index += 1
        if c == '\n':
            self.line += 1
        return c

    def peek(self, count):
        if self.index+count >= len(self.data):
            return None
        return self.data[self.index+count]

    def report_error(self, error):
        raise ParseError('{fi}({ln}): {err}'.format(err=error, ln=self.line, fi=self.name))


def read_char(f):
    return f.read()


def peek_char(f, count=0):
    return f.peek(count)


def is_space(ch):
    if ch == ' ':
        return True
    if ch == '\n':
        return True
    if ch == '\r':
        return True
    if ch == '\t':
        return True
    return False


def is_ident(first, ch):
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
    def __init__(self, name, cpp_passing):
        self.name = name
        self.cpp_passing = cpp_passing


class TypeList:
    def __init__(self):
        self.types = []

    def add_type(self, t):
        self.types.append(t)

    def add_default_types(self):
        self.add_type(Type('int8', False))
        self.add_type(Type('int16', False))
        self.add_type(Type('int32', False))
        self.add_type(Type('int64', False))
        self.add_type(Type('float', False))
        self.add_type(Type('double', False))
        self.add_type(Type('byte', False))

    def is_valid_type(self, ty):
        return ty in [x.name for x in self.types]


def get_cpp_type_from_stdint(t):
    if t =='int8':
        return 'int8_t'
    if t =='int16':
        return 'int16_t'
    if t =='int32':
        return 'int32_t'
    if t =='int64':
        return 'int64_t'
    if t =='float':
        return t
    if t =='double':
        return t
    if t =='byte':
        return 'char'
    return ''


def get_cpp_parse_from_rapidjson_helper_int(t, member, indent):
    return '{i}if(iter->value.IsInt64()==false) return false; \n' \
           '{i}else {{\n' \
           '{i}  auto gafv = iter->value.GetInt64();\n' \
           '{i}  if(gafv < std::numeric_limits<{t}>::min()) return false;\n' \
           '{i}  if(gafv > std::numeric_limits<{t}>::max()) return false;\n' \
           '{i}  c->{m}(static_cast<{t}>(gafv));\n' \
           '{i}}}\n'.format(m=member,i=indent, t=t)


def get_cpp_parse_from_rapidjson_helper_float(t, member, indent):
    return '{i}if(iter->value.IsDouble()==false) return false; \n' \
           '{i}c->{m}(iter->value.GetDouble());\n'.format(m=member,i=indent)


def get_cpp_parse_from_rapidjson(t,member, indent):
    if t =='int8':
        return get_cpp_parse_from_rapidjson_helper_int('int8_t', member, indent)
    if t =='int16':
        return get_cpp_parse_from_rapidjson_helper_int('int16_t', member, indent)
    if t =='int32':
        return get_cpp_parse_from_rapidjson_helper_int('int32_t', member, indent)
    if t =='int64':
        return '{i}if(iter->value.IsInt64()==false) return false; \n' \
               '{i}c->{m}(iter->value.GetInt64());\n'.format(m=member, i=indent, t=t)
    if t =='float':
        return get_cpp_parse_from_rapidjson_helper_float(t, member, indent)
    if t =='double':
        return get_cpp_parse_from_rapidjson_helper_float(t, member, indent)
    if t =='byte':
        return get_cpp_parse_from_rapidjson_helper_int('char', member, indent)
    return ''


def is_default_type(tn):
    tl = TypeList()
    tl.add_default_types()
    return tl.is_valid_type(tn)

def read_white_spaces(f):
    while is_space(peek_char(f)):
        read_char(f)


def read_spaces(f):
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
            while peek_char(f, 0) != '*' or peek_char(f, 1)!='/':
                read_char(f)
            star = read_char(f)
            slash = read_char(f)
            assert slash == '/'
            assert star == '*'
        else:
            return


def read_ident(f):
    read_spaces(f)
    ident = ''
    first = True
    while is_ident(first, peek_char(f)):
        ident += read_char(f)
        first = False
    if len(ident) == 0:
        raise f.report_error('expecting ident but found {}'.format(peek_char(f)))
    return ident


def read_single_char(f, ch):
    read_spaces(f)
    r = read_char(f)
    if r == ch:
        pass
    else:
        raise f.report_error('expecting char {c}, but found {r}'.format(c=ch, r=r))


class Member:
    def __init__(self, name, typename, defaultvalue):
        self.name = name
        self.typename = typename
        self.defaultvalue = defaultvalue

    def __str__(self):
        if self.defaultvalue is None:
            return '{tn} {n};'.format(n=self.name, tn=self.typename)
        else:
            return '{tn} {n} = {dv};'.format(n=self.name, tn=self.typename, dv=self.defaultvalue)


class Struct:
    def __init__(self, name):
        self.name = name
        self.members = []

    def add_member(self, member):
        self.members.append(member)

    def __str__(self):
        return 'struct {n} {{\n{mem}\n}}'.format(n=self.name, mem='\n'.join(['  ' + str(x) for x in self.members]))

def read_number(f):
    ret = ''
    while peek_char(f)[0] in '0123456789':
        ret += read_char(f)
    if len(ret) == 0:
        f.report_error('Expected number, found {}'.format(peek_char(f)))
    return ret

def read_default_value_int(f):
    return read_number(f)

def read_default_value_double(f):
    dec = read_number(f)
    read_single_char(f, '.')
    frac = read_number(f)
    return '{d}.{f}'.format(d=dec, f=frac)

def read_default_value(f, t):
    read_spaces(f)
    if t =='int8':
        return read_default_value_int(f)
    if t =='int16':
        return read_default_value_int(f)
    if t =='int32':
        return read_default_value_int(f)
    if t =='int64':
        return read_default_value_int(f)
    if t =='float':
        fl = read_default_value_double(f)
        read_single_char(f, 'f')
        return fl
    if t =='double':
        return read_default_value_double(f)
    if t =='byte':
        f.report_error('default value for byte is not yet supported')
        return ''
    return ''


def read_struct(f, tl):
    struct_name = read_ident(f)
    struct = Struct(struct_name)
    read_single_char(f, '{')
    while peek_char(f) != '}':
        ty = read_ident(f)
        name = read_ident(f)
        read_spaces(f)
        ch = peek_char(f)
        default_value = None
        if ch == '=':
            if not is_default_type(ty):
                f.report_error('structs cant have default values yet')
            read_char(f)
            default_value = read_default_value(f, ty)
        read_spaces(f)
        read_single_char(f, ';')
        mem = Member(name, ty, default_value)

        if tl.is_valid_type(ty) is False:
            f.report_error('Invalid type {t} for member {s}.{m}'.format(t=ty, s=struct_name, m=name))
        struct.add_member(mem)
        read_spaces(f)
    read_single_char(f, '}')
    tl.add_type(Type(struct_name, True))

    return struct


class File:
    def __init__(self):
        self.structs = []
        self.package_name = ''

    def __str__(self):
        package_name = ''
        if len(self.package_name) > 0:
            package_name = 'package {};\n'.format(self.package_name)
        return package_name + '\n'.join([str(x) for x in self.structs])


def read_several_structs(f):
    file = File()
    read_spaces(f)
    tl = TypeList()
    tl.add_default_types()
    while peek_char(f) is not None:
        keyword = read_ident(f)
        if keyword == 'struct':
            s = read_struct(f, tl)
            file.structs.append(s)
        elif keyword == 'package':
            read_spaces(f)
            package_name = read_ident(f)
            if len(file.package_name) > 0:
                f.report_error('tried to change package name from {old} to {new}'.format(old=file.package_name, new=package_name))
            if len(file.structs) > 0:
                f.report_error('cant change package name after adding structs')
            read_spaces(f)
            read_single_char(f, ';')
            file.package_name = package_name
        else:
            raise f.report_error('Expected struct, found unknown ident {}'.format(keyword))
        read_spaces(f) # place file marker at the next non whitespace or at eof

    return file


def to_cpp_get(name):
    return 'Get{}{}'.format(name[0].upper(), name[1:])


def to_cpp_get_mod(name):
    return 'Get{}{}Ptr'.format(name[0].upper(), name[1:])


def to_cpp_set(name):
    return 'Set{}{}'.format(name[0].upper(), name[1:])


def to_cpp_typename(name):
    return '{}_'.format(name)


def merge(iters):
    for it in iters:
        yield from it


def write_cpp(f, args, out_dir, name):
    headerguard = 'GENERATED_' + name.upper()

    source = []

    header_only = args.header_only
    write_json = args.include_json

    print(header_only)
    if header_only:
        print('headeronly')

    unique_types = set(m.typename for m in merge(s.members for s in f.structs))
    default_types = [t[0] for t in ((t, get_cpp_type_from_stdint(t)) for t in unique_types)
                     if t[1] != '' and t[0]!=t[1]
                     ]
    print('default types in file', default_types)

    with open(os.path.join(out_dir, name+'.h'), 'w', encoding='utf-8') as out:
        out.write('#ifndef {}_H\n'.format(headerguard))
        out.write('#define {}_H\n'.format(headerguard))
        out.write('\n')
        if len(default_types) > 0:
            out.write('\n')
            out.write('#include <cstdint>\n')
            out.write('\n')

        if write_json and header_only:
            out.write('#include <limits>\n')
            out.write('#include "rapidjson/document.h"\n')
            out.write('\n')

        if f.package_name != '':
            out.write('namespace {} {{\n'.format(f.package_name))
            out.write('\n')

        if len(default_types) > 0:
            out.write('\n')
            for t in default_types:
                out.write('typedef {ct} {t};\n'.format(t=t, ct=get_cpp_type_from_stdint(t)))
            out.write('\n')

        for s in f.structs:
            out.write('class {} {{\n'.format(s.name))
            out.write(' public:\n')
            # default constructor
            out.write('  {}();\n'.format(s.name))
            out.write('\n')
            common_members = [x for x in s.members if is_default_type(x.typename)]
            source.append('{n}::{n}()\n'.format(n=s.name))
            sep = ':'
            for m in common_members:
                dv = '0'
                if m.typename == 'float':
                    dv = '0.0f'
                elif m.typename == 'double':
                    dv='0.0'
                source.append('  {s} {n}({d})\n'.format(s=sep, n=to_cpp_typename(m.name), d=dv))
                sep = ','
            source.append('{}\n')
            source.append('\n')

            # reset function
            out.write('  void Reset();\n')
            out.write('\n')
            source.append('void {n}::Reset() {{\n'.format(n=s.name))
            for m in s.members:
                if is_default_type(m.typename):
                    dv = '0'
                    if m.typename == 'float':
                        dv = '0.0f'
                    elif m.typename == 'double':
                        dv = '0.0'
                    source.append('  {n} = {d};\n'.format(n=to_cpp_typename(m.name), d=dv))
                else:
                    source.append('  {n}.Reset();\n'.format(n=to_cpp_typename(m.name)))
            source.append('}\n')
            source.append('\n')

            # json
            if write_json:
                out.write('  bool ReadJsonSource(const char* const source);\n')
                out.write('\n')
                source.append('bool ReadFromJsonValue({}* c, const rapidjson::Value& value) {{\n'.format(s.name))
                source.append('  if(!value.IsObject()) return false;\n')
                source.append('  rapidjson::Value::ConstMemberIterator iter;\n')
                for m in s.members:
                    source.append('  iter = value.FindMember("{n}");\n'.format(n=m.name))
                    source.append('  if(iter != value.MemberEnd()) {\n')
                    if is_default_type(m.typename):
                        source.append(get_cpp_parse_from_rapidjson(m.typename, to_cpp_set(m.name), '    '))
                    else:
                        source.append('    if(!ReadFromJsonValue(c->{}(),iter->value)) {{ return false; }}\n'
                                      .format(to_cpp_get_mod(m.name)))
                        pass
                    source.append('  }\n')
                source.append('}\n')
                source.append('\n')
                source.append('bool {}::ReadJsonSource(const char* const source) {{\n'.format(s.name));
                source.append('  rapidjson::Document document;\n')
                source.append('  document.Parse(source);\n')
                source.append('  return ReadFromJsonValue(this, document);\n')
                source.append('}\n')
                source.append('\n')

            # member getters and setters
            for m in s.members:
                if is_default_type(m.typename):
                    out.write('  {tn} {n}() const;\n'.format(n=to_cpp_get(m.name), tn=m.typename))
                    source.append('{tn} {cn}::{n}() const {{ return {v}; }}\n'.format(n=to_cpp_get(m.name), tn=m.typename, cn=s.name, v=to_cpp_typename(m.name)))
                    out.write('  void {n}({tn} {an});\n'.format(n=to_cpp_set(m.name), an=m.name, tn=m.typename))
                    source.append('void {cn}::{n}({tn} {an}) {{ {v} = {an}; }}\n'.format(n=to_cpp_set(m.name), an=m.name, tn=m.typename, cn=s.name, v=to_cpp_typename(m.name)))
                else:
                    out.write('  const {tn}& {n}() const;\n'.format(n=to_cpp_get(m.name), tn=m.typename))
                    source.append('const {tn}& {cn}::{n}() const {{ return {v}; }}\n'.format(n=to_cpp_get(m.name), tn=m.typename, cn=s.name, v=to_cpp_typename(m.name)))
                    out.write('  {tn}* {n}();\n'.format(n=to_cpp_get_mod(m.name), tn=m.typename))
                    source.append('{tn}* {cn}::{n}() {{ return &{v}; }}\n'.format(n=to_cpp_get_mod(m.name), tn=m.typename, cn=s.name, v=to_cpp_typename(m.name)))
                    out.write('  void {n}(const {tn}& {an});\n'.format(n=to_cpp_set(m.name), an=m.name, tn=m.typename))
                    source.append('void {cn}::{n}(const {tn}& {an}) {{ {v} = {an}; }}\n'.format(n=to_cpp_set(m.name), an=m.name, tn=m.typename, cn=s.name, v=to_cpp_typename(m.name)))
                out.write('\n')
                source.append('\n')

            # member variables
            out.write(' private:\n')
            for m in s.members:
                out.write('  {tn} {n};\n'.format(n=to_cpp_typename(m.name), tn=m.typename))
            out.write('}}; // class {}\n'.format(s.name))
            if header_only and write_json:
                out.write('\n')
                out.write('bool ReadFromJsonValue({}* c, const rapidjson::Value& value);\n'.format(s.name))
            out.write('\n')

        if header_only:
            out.write('#ifdef {}_IMPLEMENTATION\n'.format(headerguard))
            out.write('\n')
            for s in source:
                out.write(s)
            out.write('#endif // {}_IMPLEMENTATION\n'.format(headerguard))

        if f.package_name != '':
            out.write('}} // namespace {}\n'.format(f.package_name))
            out.write('\n')
        out.write('\n')
        out.write('#endif  // {}_H\n'.format(headerguard))

    if header_only == False:
        with open(os.path.join(out_dir, name+'.cc'), 'w', encoding='utf-8') as out:
            out.write('#include "{}.h"\n'.format(name))
            out.write('\n')
            out.write('#include <limits>\n')
            out.write('#include "rapidjson/document.h"\n')
            out.write('\n')

            if f.package_name != '':
                out.write('namespace {} {{\n'.format(f.package_name))
                out.write('\n')

            for s in source:
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
        write_cpp(s, args, args.output_folder, os.path.splitext(os.path.basename(file.name))[0] if args.name is None else args.name)
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

    gen_parser = sub.add_parser('generate', help='generate a game format parser', aliases=['gen'], fromfile_prefix_chars='@')
    gen_parser.add_argument('language', type=EnumType(Language), help='the language')
    gen_parser.add_argument('input', type=argparse.FileType('r'), help='the source gaf file')
    gen_parser.add_argument('output_folder', help='the output directory')
    gen_parser.add_argument('--debug', action='store_const', const=True, default=False, help='debug gaf')
    gen_parser.add_argument('--name', help='use this name instead of the autogenerated name')
    gen_parser.add_argument('--header-only', action='store_const', const=True, default=False, help='header only implementation')
    gen_parser.add_argument('--include-json', action='store_const', const=True, default=False, help='include rapid json implementation')
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
