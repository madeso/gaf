#!/usr/bin/env python3

import typing

from gaf_types import StandardType, is_default_type, Struct, TypeList, File, Type, Member, Enum


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


def read_default_value_string(f: CharFile) -> str:
    read_single_char(f, '"')
    r = '"'
    while peek_char(f) is not '"':
        ch = read_char(f)
        r += ch
        if ch == '\\':
            r += read_char(f)

    read_single_char(f, '"')
    r += '"'
    return r


def read_default_value(f: CharFile, t: Type, fi: File) -> str:
    read_spaces(f)

    p = peek_char(f)
    if is_ident(True, p):
        ident = read_ident(f)

        if t.standard_type == StandardType.bool:
            if ident == 'true':
                return 'true'
            if ident == 'false':
                return 'false'
        if t.is_enum:
            e = fi.find_enum(t.name)
            if e is None:
                f.report_error('BUG: failed to find enum {t} while loooking up {n}'.format(n=ident, t=t.name))
                return ''
            if ident in e.values:
                return ident
            else:
                f.report_error('{n} is not a valid enum value of {t}'.format(n=ident, t=t.name))
                return ''
        c = fi.find_constant(ident, t)
        if c is None:
            f.report_error('failed to find constant named {n} with a type {t}'.format(n=ident, t=t.name))
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
    if t.standard_type == StandardType.string:
        return read_default_value_string(f)
    if t.standard_type == StandardType.double:
        return read_default_value_double(f)
    if t.standard_type == StandardType.byte:
        f.report_error('default value for byte is not yet supported')
        return ''
    return ''


def read_struct(f: CharFile, type_list: TypeList, fi: File) -> Struct:
    struct_name = read_ident(f)
    struct = Struct(struct_name)

    forward = fi.find_struct(struct_name)
    if forward is None:
        type_list.add_type(Type(StandardType.INVALID, struct_name, False))
    else:
        struct = forward

    read_spaces(f)
    ch = peek_char(f)

    if ch == ';':
        read_char(f)
    elif ch == '{':
        read_char(f)
        if struct.is_defined:
            f.report_error('structs {} has already been defined'.format(struct_name))
        struct.is_defined = True
        while peek_char(f) != '}':
            ty = read_ident(f)

            is_optional = False
            read_spaces(f)
            ch = peek_char(f)
            if ch == '?':
                is_optional = True
                read_char(f)
                read_spaces(f)
                ch = peek_char(f)

            name = read_ident(f)
            if type_list.is_valid_type(ty) is False:
                f.report_error('Invalid type {t} for member {s}.{m}'.format(t=ty, s=struct_name, m=name))
            valid_type = type_list.get_type(ty) if type_list.is_valid_type(ty) else StandardType.int32
            s = fi.find_struct(ty)
            if s is not None:
                if not s.is_defined:
                    f.report_error('Struct {t} is not defined yet for {s}.{m}. Define or use optional'.format(t=ty, s=struct_name, m=name))
            mem = Member(name, valid_type)
            mem.is_optional = is_optional

            read_spaces(f)
            ch = peek_char(f)

            if not is_optional:
                if ch == '[':
                    read_char(f)
                    read_spaces(f)
                    read_single_char(f, ']')
                    mem.is_dynamic_array = True
                    mem.defaultvalue = None

                    read_spaces(f)
                    ch = peek_char(f)
                if ch == '?':
                    read_char(f)
                    read_spaces(f)
                    ch = peek_char(f)
                    mem.missing_is_fail = False

            if ch == '=':
                if not is_default_type(ty) and not valid_type.is_enum:
                    f.report_error('structs cant have default values yet')
                if mem.is_dynamic_array:
                    f.report_error('dynamic arrays cant have default values yet')
                read_char(f)
                mem.defaultvalue = read_default_value(f, valid_type, fi)
            read_spaces(f)
            read_single_char(f, ';')

            struct.add_member(mem)
            read_spaces(f)
        read_single_char(f, '}')

    return struct


def read_enum(f: CharFile, type_list: TypeList) -> Enum:
    enum_name = read_ident(f)
    e = Enum(enum_name)

    read_spaces(f)
    if peek_char(f) == ':':
        read_char(f)
        type_name = read_ident(f)
        if not type_list.is_valid_type(type_name):
            f.report_error('{enum} has a invalid type for size type: {type}'.format(type=type_name, enum=enum_name))
        t = type_list.get_type(type_name)
        if not t.is_int:
            f.report_error('{enum} has a non-int for size type: {type}'.format(type=type_name, enum=enum_name))
        e.type = t

    read_spaces(f)
    read_single_char(f, '{')
    while peek_char(f) != '}':
        name = read_ident(f)

        if name in e.values:
            f.report_error('{prop} already specifed in {enum}'.format(prop=name, enum=enum_name))

        e.values.append(name)

        read_spaces(f)
        ch = peek_char(f)

        if ch == ',':
            read_char(f)
            read_spaces(f)
    read_single_char(f, '}')

    type_list.add_type(Type(StandardType.INVALID, enum_name, is_int=False, is_enum=True, default_value=e.values[0]))

    return e


def read_several_structs(f: CharFile) -> File:
    file = File()
    read_spaces(f)
    type_list = TypeList()
    type_list.add_default_types()
    while peek_char(f) is not None:
        keyword = read_ident(f)
        if keyword == 'struct':
            s = read_struct(f, type_list, file)
            if file.find_struct(s.name) is None:
                file.structs.append(s)

            if s.is_defined:
                file.structs_defined.append(s)
            else:
                file.typedefs.append(s)
        elif keyword == 'enum':
            e = read_enum(f, type_list)
            file.enums.append(e)
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
            raise f.report_error('Expected struct, enum, package or const. Found unknown ident {}'.format(keyword))
        read_spaces(f)  # place file marker at the next non whitespace or at eof

    return file
