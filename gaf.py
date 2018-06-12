#!/usr/bin/env python3
# GAme Format Parser


from enum import Enum
import enum
import os
import typing
import argparse

from gaf_cpp import write_cpp
from gaf_parse import CharFile, read_several_structs, ParseError
from gaf_types import OutputOptions, CppEnumStyle, CppJsonReturn


class EnumType(object):
    """Factory for creating enum object types
    """

    def __init__(self, enumclass):
        self.enums = dict()
        self.name = enumclass.__name__
        for e in enumclass:
            self.enums[e.name.lower()] = e

    def __call__(self, astring):
        try:
            return self.enums[astring.lower()]
        except KeyError:
            msg = ', '.join([t.name.lower() for t in self.enums])
            msg = '%s: use one of {%s}' % (self.name, msg)
            raise argparse.ArgumentTypeError(msg)

    def __repr__(self):
        astr = ', '.join([t.name.lower() for t in self.enums])
        return '%s(%s)' % (self.enums.__name__, astr)


@enum.unique
class Language(Enum):
    CPP = object()


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
        e = args.enum
        r = args.json_return
        opt = OutputOptions(header_only=args.header_only, write_json=args.include_json,
                            enum_style=CppEnumStyle.PrefixEnum if e is None else e,
                            prefix=args.prefix if args.prefix is not None else '',
                            json_return=CppJsonReturn.Char if r is None else r,
                            write_imgui=args.include_imgui,
                            imgui_header=args.imgui_header if args.imgui_header is not None else '"imgui.h"')
        write_cpp(s, opt, args.output_folder,
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
    parser = argparse.ArgumentParser(description='GAme Format parser', fromfile_prefix_chars='@')
    parser.set_defaults(func=None)
    sub = parser.add_subparsers(help='sub-command help')

    gen_parser = sub.add_parser('generate', help='generate a game format parser', aliases=['gen'],
                                fromfile_prefix_chars='@')
    gen_parser.add_argument('language', type=EnumType(Language), help='the language')
    gen_parser.add_argument('input', type=argparse.FileType('r'), help='the source gaf file')
    gen_parser.add_argument('output_folder', help='the output directory')
    gen_parser.add_argument('--enum', type=EnumType(CppEnumStyle), help='how c++ will generate enums')
    gen_parser.add_argument('--json_return', type=EnumType(CppJsonReturn), help='how c++ json loading will return errors')
    gen_parser.add_argument('--debug', action='store_const', const=True, default=False, help='debug gaf')
    gen_parser.add_argument('--name', help='use this name instead of the autogenerated name')
    gen_parser.add_argument('--prefix', help='add this prefix to the autogenerated file name')
    gen_parser.add_argument('--header-only', action='store_const', const=True, default=False,
                            help='header only implementation')
    gen_parser.add_argument('--include-json', action='store_const', const=True, default=False,
                            help='include rapid json implementation')
    gen_parser.add_argument('--include-imgui', action='store_const', const=True, default=False,
                            help='include dear imgui implementation')
    gen_parser.add_argument('--imgui-header', help='use this header instead of the standard imgui')
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
