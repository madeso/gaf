#!/usr/bin/env python3
# GAme Format Parser

from enum import Enum
import enum
import os
import argparse

from gaf_cpp import write_cpp
from gaf_parse import CharFile, read_several_structs, ParseError
from gaf_types import OutputOptions


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
    opt = OutputOptions(write_json=args.include_json,
                        prefix='gaf_',
                        write_imgui=args.include_imgui,
                        imgui_headers=args.imgui_headers if args.imgui_headers is not None else ['"imgui.h"'],
                        imgui_add=args.imgui_add if args.imgui_add is not None else '"Add"',
                        imgui_remove=args.imgui_remove if args.imgui_remove is not None else '"Remove"')
    write_cpp(s, opt, args.output_folder, os.path.splitext(os.path.basename(file.name))[0])


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
    gen_parser.add_argument('input', type=argparse.FileType('r'), help='the source gaf file')
    gen_parser.add_argument('output_folder', help='the output directory')
    gen_parser.add_argument('--debug', action='store_const', const=True, default=False, help='debug gaf')
    gen_parser.add_argument('--include-json', action='store_const', const=True, default=False,
                            help='include rapid json implementation')
    gen_parser.add_argument('--include-imgui', action='store_const', const=True, default=False,
                            help='include dear imgui implementation')
    gen_parser.add_argument('--imgui-headers', nargs='+', help='use this header instead of the standard imgui')
    gen_parser.add_argument('--imgui-add', help='the imgui add item button text')
    gen_parser.add_argument('--imgui-remove', help='the imgui remove item button text')
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
