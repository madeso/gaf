#!/usr/bin/env python3
# GAme Format Parser
import os
import typing
import argparse

from gaf_cpp import CppPlugin, RapidJsonPlugin, ImguiPlugin
from gaf_parse import CharFile, read_several_structs, ParseError
from gaf_types import Plugin


def on_generate_command(args, plugins: typing.Dict[str, Plugin]):
    file = CharFile(args.input)
    if args.debug:
        parsed_file = read_several_structs(file)
    else:
        try:
            parsed_file = read_several_structs(file)
        except ParseError as parse_error:
            print(parse_error.message)
            return
    name = os.path.splitext(os.path.basename(file.name))[0]
    print(name)
    plugin = plugins[args.command]
    plugin.run_plugin(parsed_file, name, args)


def on_display_command(args, plugins):
    if args.debug:
        s = read_several_structs(CharFile(args.file))
    else:
        try:
            s = read_several_structs(CharFile(args.file))
        except ParseError as p:
            print(p.message)
            return
    print(s)


def main(plugins_list: typing.List[Plugin]):
    parser = argparse.ArgumentParser(description='GAme Format parser', fromfile_prefix_chars='@')
    parser.set_defaults(func=None)
    sub = parser.add_subparsers(help='sub-command help')

    plugins = {plugin.get_name(): plugin for plugin in plugins_list}
    plugin_names = [plugin.get_name() for plugin in plugins_list]

    gen_parser = sub.add_parser('generate', help='generate a game format parser',
                                fromfile_prefix_chars='@')
    gen_parser.add_argument('input', type=argparse.FileType('r'), help='the source gaf file')
    gen_parser.add_argument('output_folder', help='the output directory')
    gen_parser.add_argument('command', choices=plugin_names, help='the command/plugin to run')
    gen_parser.add_argument('--debug', action='store_const', const=True, default=False, help='debug gaf')
    for plugin in plugins_list:
        plugin.add_arguments(gen_parser)
    
    gen_parser.set_defaults(func=on_generate_command)

    dis_parser = sub.add_parser('display', help='generate a game format parser')
    dis_parser.add_argument('file', type=argparse.FileType('r'), help='the source gaf file')
    dis_parser.add_argument('--debug', action='store_const', const=False, default=True, help='debug gaf')
    dis_parser.set_defaults(func=on_display_command)

    args = parser.parse_args()
    if args.func is not None:
        args.func(args, plugins)


if __name__ == "__main__":
    main([CppPlugin(), RapidJsonPlugin(), ImguiPlugin()])
