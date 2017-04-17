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


def get_cpp_parse_from_rapidjson_helper_int(t, member, indent, name):
    return '{i}if(iter->value.IsInt64()==false) return "read value for {n} was not a integer"; \n' \
           '{i}else {{\n' \
           '{i}  auto gafv = iter->value.GetInt64();\n' \
           '{i}  if(gafv < std::numeric_limits<{t}>::min()) return "read value for {n} was to low";\n' \
           '{i}  if(gafv > std::numeric_limits<{t}>::max()) return "read value for {n} was to high";\n' \
           '{i}  c->{m}(static_cast<{t}>(gafv));\n' \
           '{i}}}\n'.format(m=member,i=indent, t=t, n=name)


def get_cpp_parse_from_rapidjson_helper_float(t, member, indent, name):
    return '{i}if(iter->value.IsDouble()==false) return "read value for {n} was not a double"; \n' \
           '{i}c->{m}(iter->value.GetDouble());\n'.format(m=member,i=indent, n=name)


def get_cpp_parse_from_rapidjson(t,member, indent, name):
    if t =='int8':
        return get_cpp_parse_from_rapidjson_helper_int('int8_t', member, indent, name)
    if t =='int16':
        return get_cpp_parse_from_rapidjson_helper_int('int16_t', member, indent, name)
    if t =='int32':
        return get_cpp_parse_from_rapidjson_helper_int('int32_t', member, indent, name)
    if t =='int64':
        return '{i}if(iter->value.IsInt64()==false) return "read value for {n} was not a integer"; \n' \
               '{i}c->{m}(iter->value.GetInt64());\n'.format(m=member, i=indent, t=t, n=name)
    if t =='float':
        return get_cpp_parse_from_rapidjson_helper_float(t, member, indent, name)
    if t =='double':
        return get_cpp_parse_from_rapidjson_helper_float(t, member, indent, name)
    if t =='byte':
        return get_cpp_parse_from_rapidjson_helper_int('char', member, indent, name)
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


class ArrayData:
    def __init__(self):
        self.total_size_number = None
        self.total_size_type = None
        self.current_size = None


class Member:
    def __init__(self, name, typename):
        self.name = name
        self.typename = typename
        self.defaultvalue = None
        self.array = None

        # start with a default value for some built in types
        if is_default_type(self.typename):
            self.defaultvalue = '0'
            if self.typename == 'float':
                self.defaultvalue = '0.0f'
            elif self.typename == 'double':
                self.defaultvalue = '0.0'

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


class Constant:
    def __init__(self, n, t, v):
        self.name = n
        self.type = t
        self.value = v


class File:
    def __init__(self):
        self.structs = []
        self.constants = []
        self.package_name = ''

    def __str__(self):
        package_name = ''
        if len(self.package_name) > 0:
            package_name = 'package {};\n'.format(self.package_name)
        return package_name + '\n'.join([str(x) for x in self.structs])

    def add_constant(self, n, t, v):
        self.constants.append(Constant(n, t, v))

    def find_constant(self, name, ty):
        for c in self.constants:
            if ty is None:
                if c.name == name:
                    return c
            elif c.name == name and c.type == ty:
                return c
        return None


def is_number(n):
    return n in '0123456789'


def read_number(f):
    ret = ''
    while is_number(peek_char(f)[0]):
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


def read_default_value(f, t, fi):
    read_spaces(f)

    p = peek_char(f)
    if is_ident(True, p):
        ident = read_ident(f)
        c = fi.find_constant(ident, t)
        if c is None:
            f.report_error('failed to find constant named {n} with a type {t}'.format(n=ident, t=t))
            return ''
        return c.value

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


def read_array(f, fi):
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
            f.report_error('unexpected character in array expression: {}', c)
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



def read_struct(f, tl, fi):
    struct_name = read_ident(f)
    struct = Struct(struct_name)
    read_single_char(f, '{')
    while peek_char(f) != '}':
        ty = read_ident(f)
        name = read_ident(f)
        read_spaces(f)
        ch = peek_char(f)
        mem = Member(name, ty)

        if ch == '[':
            array = read_array(f, fi)

            mem.array = array

            read_spaces(f)
            ch = peek_char(f)

        if ch == '=':
            if not is_default_type(ty):
                f.report_error('structs cant have default values yet')
            read_char(f)
            mem.defaultvalue = read_default_value(f, ty, fi)
        read_spaces(f)
        read_single_char(f, ';')

        if tl.is_valid_type(ty) is False:
            f.report_error('Invalid type {t} for member {s}.{m}'.format(t=ty, s=struct_name, m=name))
        struct.add_member(mem)
        read_spaces(f)
    read_single_char(f, '}')
    tl.add_type(Type(struct_name, True))

    return struct


def read_several_structs(f):
    file = File()
    read_spaces(f)
    tl = TypeList()
    tl.add_default_types()
    while peek_char(f) is not None:
        keyword = read_ident(f)
        if keyword == 'struct':
            s = read_struct(f, tl, file)
            file.structs.append(s)
        elif keyword == 'const':
            type = read_ident(f)
            name = read_ident(f)
            read_spaces(f)
            read_single_char(f, '=')
            val = read_default_value(f, type, file)
            read_single_char(f, ';')
            file.add_constant(name, type, val)
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
            raise f.report_error('Expected struct, package or const. Found unknown ident {}'.format(keyword))
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


def add_json_to_string_for_cpp(write_json, header_only, out, source):
    if write_json:
        if header_only:
            out.write('const char* const JsonToStringError(rapidjson::ParseErrorCode);\n')
            out.write('\n')
        source.append('const char* const JsonToStringError(rapidjson::ParseErrorCode err) {\n')
        source.append('  switch(err) {\n')
        source.append('  case rapidjson::kParseErrorNone: return nullptr;\n')
        source.append('  case rapidjson::kParseErrorDocumentEmpty: return "JSON: The document is empty.";\n')
        source.append('  case rapidjson::kParseErrorDocumentRootNotSingular: return "JSON: The document root must not follow by other values.";\n')
        source.append('  case rapidjson::kParseErrorValueInvalid: return "JSON: Invalid value.";\n')
        source.append('  case rapidjson::kParseErrorObjectMissName: return "JSON: Missing name for a object member.";\n')
        source.append('  case rapidjson::kParseErrorObjectMissColon: return "JSON: Missing a colon after a name of object member.";\n')
        source.append('  case rapidjson::kParseErrorObjectMissCommaOrCurlyBracket: return "JSON: Missing a comma or } after an object member.";\n')
        source.append('  case rapidjson::kParseErrorArrayMissCommaOrSquareBracket: return "JSON: Missing a comma or ] after an array element.";\n')
        source.append('  case rapidjson::kParseErrorStringUnicodeEscapeInvalidHex: return "JSON: Incorrect hex digit after \\\\u escape in string.";\n')
        source.append('  case rapidjson::kParseErrorStringUnicodeSurrogateInvalid: return "JSON: The surrogate pair in string is invalid.";\n')
        source.append('  case rapidjson::kParseErrorStringEscapeInvalid: return "JSON: Invalid escape character in string.";\n')
        source.append('  case rapidjson::kParseErrorStringMissQuotationMark: return "JSON: Missing a closing quotation mark in string.";\n')
        source.append('  case rapidjson::kParseErrorStringInvalidEncoding: return "JSON: Invalid encoding in string.";\n')
        source.append('  case rapidjson::kParseErrorNumberTooBig: return "JSON: Number too big to be stored in double.";\n')
        source.append('  case rapidjson::kParseErrorNumberMissFraction: return "JSON: Miss fraction part in number.";\n')
        source.append('  case rapidjson::kParseErrorNumberMissExponent: return "JSON: Miss exponent in number.";\n')
        source.append('  case rapidjson::kParseErrorTermination: return "JSON: Parsing was terminated.";\n')
        source.append('  case rapidjson::kParseErrorUnspecificSyntaxError: return "JSON: Unspecific syntax error.";\n')
        source.append('  }\n')
        source.append('  return "undefined erropr";\n')
        source.append('}\n')
        source.append('\n')


def write_reset_function_for_cpp(out, source, s):
    out.write('  void Reset();\n')
    out.write('\n')
    source.append('void {n}::Reset() {{\n'.format(n=s.name))
    for m in s.members:
        if m.defaultvalue is not None:
            source.append('  {n} = {d};\n'.format(n=to_cpp_typename(m.name), d=m.defaultvalue))
        else:
            source.append('  {n}.Reset();\n'.format(n=to_cpp_typename(m.name)))
    source.append('}\n')
    source.append('\n')

def write_json_source_for_cpp(write_json, source, out, s):
    if write_json:
        out.write('  const char* const ReadJsonSource(const char* const source);\n')
        out.write('\n')
        source.append('const char* const ReadFromJsonValue({}* c, const rapidjson::Value& value) {{\n'.format(s.name))
        source.append('  if(!value.IsObject()) return "tried to read {} but value was not a object";\n'.format(s.name))
        source.append('  rapidjson::Value::ConstMemberIterator iter;\n')
        for m in s.members:
            source.append('  iter = value.FindMember("{n}");\n'.format(n=m.name))
            source.append('  if(iter != value.MemberEnd()) {\n')
            if is_default_type(m.typename):
                source.append(get_cpp_parse_from_rapidjson(m.typename, to_cpp_set(m.name), '    ', m.name))
            else:
                source.append('   {{  const char* const r = ReadFromJsonValue(c->{}(),iter->value); if(r!=nullptr) {{ return r; }} }}\n'
                              .format(to_cpp_get_mod(m.name)))
            source.append('  }\n')
            source.append('  else {\n')
            source.append('    return "missing {} in json object";\n'.format(m.name))
            source.append('  }\n')
        source.append('  return nullptr;\n')
        source.append('}\n')
        source.append('\n')
        source.append('const char* const {}::ReadJsonSource(const char* const source) {{\n'.format(s.name))
        source.append('  rapidjson::Document document;\n')
        source.append('  document.Parse(source);\n')
        source.append('  const char* const err = JsonToStringError(document.GetParseError());\n')
        source.append('  if(err != nullptr ) {return err;}\n')
        source.append('  return ReadFromJsonValue(this, document);\n')
        source.append('}\n')
        source.append('\n')


def write_setter_and_getter_for_cpp(s, source, out):
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


def write_member_variables_for_cpp(out, s):
    out.write(' private:\n')
    for m in s.members:
        out.write('  {tn} {n};\n'.format(n=to_cpp_typename(m.name), tn=m.typename))
    out.write('}}; // class {}\n'.format(s.name))


def write_default_constructor_for_cpp(s, out, source):
    out.write('  {}();\n'.format(s.name))
    out.write('\n')
    common_members = [x for x in s.members if x.defaultvalue is not None]
    source.append('{n}::{n}()\n'.format(n=s.name))
    sep = ':'
    for m in common_members:
        source.append('  {s} {n}({d})\n'.format(s=sep, n=to_cpp_typename(m.name), d=m.defaultvalue))
        sep = ','
    source.append('{}\n')
    source.append('\n')


def write_cpp(f, args, out_dir, name):
    headerguard = 'GENERATED_' + name.upper()

    source = []

    header_only = args.header_only
    write_json = args.include_json

    unique_types = set(m.typename for m in merge(s.members for s in f.structs))
    default_types = [t[0] for t in ((t, get_cpp_type_from_stdint(t)) for t in unique_types)
                     if t[1] != '' and t[0]!=t[1]
                     ]

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

        add_json_to_string_for_cpp(write_json, header_only, out, source)

        for s in f.structs:
            out.write('class {} {{\n'.format(s.name))
            out.write(' public:\n')
            write_default_constructor_for_cpp(s, out, source)

            write_reset_function_for_cpp(out, source, s)
            write_json_source_for_cpp(write_json, source, out, s)
            write_setter_and_getter_for_cpp(s, source, out)
            write_member_variables_for_cpp(out, s)

            if header_only and write_json:
                out.write('\n')
                out.write('const char* const ReadFromJsonValue({}* c, const rapidjson::Value& value);\n'.format(s.name))
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
            if write_json:
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
