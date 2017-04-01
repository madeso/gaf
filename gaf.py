#!/usr/bin/env python3
# GAme Format Parser


class ParseError(Exception):
    def __init__(self, message):
        self.message = message


class CharFile:
    def __init__(self, file):
        self.data = file.read()
        self.index = 0

    def read(self):
        c = self.data[self.index]
        self.index += 1
        return c

    def peek(self):
        return self.data[self.index]


def read_char(file):
    return file.read()


def peek_char(file):
    return file.peek()


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
    if ch >= 'a' and ch <= 'z':
        return True
    if ch >= 'A' and ch <= 'Z':
        return True
    if ch == '_':
        return True
    if first is False:
        if ch > '0' and ch < '9':
            return True
    return False


def is_valid_type(type):
    if type == 'int8':
        return True
    if type == 'int16':
        return True
    if type == 'int32':
        return True
    if type == 'int64':
        return True
    if type == 'float':
        return True
    if type == 'double':
        return True
    if type == 'byte':
        return True
    return False


def read_spaces(file):
    while is_space(peek_char(file)):
        read_char(file)


def read_ident(file):
    read_spaces(file)
    ident = ''
    first = True
    while is_ident(first, peek_char(file)):
        ident += read_char(file)
        first = False
    if len(ident) == 0:
        raise ParseError('expecting ident but found {}'.format(peek_char(file)))
    return ident


def read_single_char(file, ch):
    read_spaces(file)
    r = read_char(file)
    if r == ch:
        pass
    else:
        raise ParseError('expecting char {c}, but found {r}'.format(c=ch, r=r))


def read_struct(file):
    struct = read_ident(file)
    if struct != 'struct':
        raise ParseError('expected struct found ident {}'.format(struct))
    typename = read_ident(file)
    read_single_char(file, '{')
    while peek_char(file) != '}':
        type = read_ident(file)
        if is_valid_type(type) is False:
            raise ParseError('Inavlid type {}'.format(type))
        name = read_ident(file)
        # todo: add default value
        read_single_char(file, ';')
        read_spaces(file)
    read_single_char(file, '}')


def on_generate_command(args):
    read_struct(CharFile(args.file))


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
