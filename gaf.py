#!/usr/bin/env python3
# GAme Format Parser


class ParseError(Exception):
    def __init__(self, message):
        self.message = message


class CharFile:
    def __init__(self, f):
        self.data = f.read()
        self.index = 0

    def read(self):
        c = self.data[self.index]
        self.index += 1
        return c

    def peek(self):
        return self.data[self.index]


def read_char(f):
    return f.read()


def peek_char(f):
    return f.peek()


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


def is_valid_type(ty):
    if ty == 'int8':
        return True
    if ty == 'int16':
        return True
    if ty == 'int32':
        return True
    if ty == 'int64':
        return True
    if ty == 'float':
        return True
    if ty == 'double':
        return True
    if ty == 'byte':
        return True
    return False


def read_spaces(f):
    while is_space(peek_char(f)):
        read_char(f)


def read_ident(f):
    read_spaces(f)
    ident = ''
    first = True
    while is_ident(first, peek_char(f)):
        ident += read_char(f)
        first = False
    if len(ident) == 0:
        raise ParseError('expecting ident but found {}'.format(peek_char(f)))
    return ident


def read_single_char(f, ch):
    read_spaces(f)
    r = read_char(f)
    if r == ch:
        pass
    else:
        raise ParseError('expecting char {c}, but found {r}'.format(c=ch, r=r))


class Member:
    def __init__(self, name, typename):
        self.name = name
        self.typename = typename


class Struct:
    def __init__(self, name):
        self.name = name
        self.members = []

    def add_member(self, member):
        self.members.append(member)


def read_struct(f):
    struct_keyword = read_ident(f)
    if struct_keyword != 'struct':
        raise ParseError('expected struct found ident {}'.format(struct_keyword))
    struct_name = read_ident(f)
    struct = Struct(struct_name)
    read_single_char(f, '{')
    while peek_char(f) != '}':
        ty = read_ident(f)
        if is_valid_type(ty) is False:
            raise ParseError('Inavlid type {}'.format(ty))
        name = read_ident(f)
        mem = Member(name, ty)
        # todo: add default value
        read_single_char(f, ';')
        struct.add_member(mem)
        read_spaces(f)
    read_single_char(f, '}')
    return struct


def on_generate_command(args):
    s = read_struct(CharFile(args.file))
    print(s)


def main():
    import argparse
    parser = argparse.ArgumentParser(description='GAme Format parser')
    sub = parser.add_subparsers(help='sub-command help')

    gen_parser = sub.add_parser('generate', help='generate a game format parser', aliases=['gen'])
    gen_parser.add_argument('file', type=argparse.FileType('r'))
    gen_parser.set_defaults(func=on_generate_command)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
