#!/usr/bin/env python3
# GAF test, please ignore

import os
import sys
import subprocess
import shutil
import itertools


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def ensure_folder_exist(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)


class Code:
    def __init__(self, name, gaf=None, test=None, use_enum: bool = False, test_underscore: bool = False):
        self.name = name
        self.gaf = gaf if gaf is not None else name+'.c'
        self.test = test if test is not None else name+'.cc'
        self.use_enum = use_enum
        self.test_underscore = test_underscore


def remove_folder(d):
    if os.path.exists(d):
        shutil.rmtree(d)


def print_result(root, cmake_result, make_result, app_result):
    with open(os.path.join(root, 'cmake_result.txt'), 'w') as f:
        f.write(cmake_result)
    with open(os.path.join(root, 'make_result.txt'), 'w') as f:
        f.write(make_result)
    with open(os.path.join(root, 'app_result.txt'), 'w') as f:
        f.write(app_result)


class CodeRun:
    def __init__(self, json_test: bool, header_only_test: bool):
        codename = ''
        if json_test:
            codename += '_json'
        if header_only_test:
            codename += '_headeronly'
        self.json_test = json_test
        self.header_only_test = header_only_test
        self.codename = codename.strip('_')


def all_coderuns():
    return [CodeRun(json_test, header_only_test)
            for json_test, header_only_test in itertools.product(
            [True, False], [True, False])]


def main():
    code_examples = [
        Code('defaultvalues'),
        Code('constants', test='defaultvalues.cc'),
        Code('comments1', test='comments.cc'),
        Code('comments2', test='comments.cc'),
        Code('comments3', test='comments.cc'),
        Code('onestruct'),
        Code('underscore', gaf='onestruct.c', test='underscore_onestruct.cc', test_underscore = True),
        Code('package'),
        Code('twostructs'),
        Code('optional'),
        Code('arrays'),
        Code('standard_types'),
        Code('enum', use_enum=True)
    ]

    import argparse
    parser = argparse.ArgumentParser(description='test script')
    parser.add_argument('example', nargs='+', help='the examples to test', choices=['all'] + [c.name for c in code_examples])
    parser.add_argument('--only', nargs='+', default=None, help='the examples to test', choices=[x.codename for x in all_coderuns()])
    parser.add_argument('--rv', type=int, help='change the return value')
    args = parser.parse_args()
    if 'all' not in args.example:
        code_examples = [c for c in code_examples if c.name in args.example]
    root_folder = os.getcwd()
    print('root', root_folder)

    rvs = ['Char', 'Bool', 'String']
    if args.rv is not None:
        rvs = [rvs[args.rv]]

    build_folder = os.path.join(root_folder, 'build')
    remove_folder(build_folder)

    ensure_folder_exist(build_folder)

    total_errors = 0
    total_tests = 0
    total_oks = 0

    coderuns = all_coderuns()

    for code in code_examples:
        for run in coderuns:
            for rv in rvs:
                enum_values = ['EnumClass', 'NamespaceEnum', 'PrefixEnum'] if code.use_enum else ['PrefixEnum']
                for ev in enum_values:
                    if args.only is None or run.codename in args.only:
                        codename = code.name
                        if len(run.codename):
                            codename += '_' + run.codename

                        if code.use_enum:
                            codename += '_' + ev

                        codename += '_' + rv

                        print()
                        print('----------------------------------------------------------------')
                        print('--- {}'.format(codename))
                        print('----------------------------------------------------------------')
                        code_root_folder = os.path.join(build_folder, codename)

                        remove_folder(code_root_folder)
                        ensure_folder_exist(code_root_folder)
                        with open(os.path.join(code_root_folder, 'cmdlineargs.txt'), 'w') as cmd:
                            if run.json_test:
                                cmd.write('--include-json\n')
                            if run.header_only_test:
                                cmd.write('--header-only\n')

                            cmd.write('--enum\n')
                            cmd.write(ev + '\n')
                            cmd.write('--json_return\n')
                            cmd.write(rv + '\n')

                        cpp_version = '11'
                        if ev == 'EnumClass':
                            cpp_version = '11'

                        with open(os.path.join(code_root_folder, 'CMakeLists.txt'), 'w') as cmake_file:
                            test_cpp_folder = os.path.join(root_folder, 'test-cpp')
                            underscore = 'SET(Gaf_CUSTOM_PREFIX gaf_)' if code.test_underscore else 'SET(Gaf_CUSTOM_NAME mygaf)'
                            cmake_file.write('''
                            cmake_minimum_required(VERSION 3.1)
                            project(gaf)
                            set (CMAKE_CXX_STANDARD {cppversion})
                            set(CMAKE_CXX_STANDARD_REQUIRED ON)
                            if (MSVC)
                              add_compile_options(/W4)
                            else()
                              add_compile_options(-Wall -Wextra -Wpedantic)
                            endif()
                            SET(GAF_TEST_HEADER_ONLY {headeronly})
                            SET(GAF_TEST_JSON {json})
                            SET(GAF_ENUM_STYLE_{enum} Yes)
                            SET(GAF_JSON_RETURN_{rv} Yes)
                            CONFIGURE_FILE("{config}" "${{PROJECT_BINARY_DIR}}/config.h")
                            include_directories("${{PROJECT_BINARY_DIR}}")
        
                            include_directories(SYSTEM {external}/catch)
                            include_directories(SYSTEM {external}/rapidjson-1.1.0/include)
                            include({root}/gaf.cmake)
                            {custom_underscore}
                            SET(Gaf_CUSTOM_ARGUMENTS_FROM_FILE {coderoot}/cmdlineargs.txt)
                            GAF_GENERATE_CPP(GAF_SOURCES GAF_HEADERS {root}/examples/{gaf})
                            include_directories(${{CMAKE_CURRENT_BINARY_DIR}})
                            add_executable(app {cpp} {gaf_sources}${{GAF_HEADERS}})
                            '''.format(
                                cppversion=cpp_version,
                                gaf=code.gaf,
                                cpp= os.path.join(test_cpp_folder, code.test),
                                config= os.path.join(test_cpp_folder, 'config_in.h'),
                                external=os.path.join(test_cpp_folder, 'external'),
                                root=root_folder,
                                enum=ev,
                                rv=rv,
                                coderoot=code_root_folder,
                                headeronly = '1' if run.header_only_test else '0',
                                json = '1' if run.json_test else '0',
                                gaf_sources = '' if run.header_only_test else '${GAF_SOURCES} ',
                                custom_underscore=underscore
                            ))
                        code_build_folder = os.path.join(code_root_folder, 'build')
                        ensure_folder_exist(code_build_folder)
                        cmake_result = ''
                        make_result = ''
                        app_result = ''
                        try:
                            print('running cmake')
                            cmake_result = subprocess.check_output(['cmake', '..'], stderr=subprocess.STDOUT, universal_newlines=True, cwd=code_build_folder)
                            print('running make')
                            make_result = subprocess.check_output(['make'], stderr=subprocess.STDOUT, universal_newlines=True, cwd=code_build_folder)
                            print('running code')
                            code_run_folder = os.path.join(code_build_folder, 'run')
                            ensure_folder_exist(code_run_folder)
                            app_result = subprocess.check_output(['../app'], stderr=subprocess.STDOUT, universal_newlines=True, cwd=code_run_folder)
                            print_result(code_root_folder, cmake_result, make_result, app_result)
                            total_oks += 1
                        except subprocess.CalledProcessError as exc:
                            eprint('Status: FAIL')
                            eprint('Error code: {}'.format(exc.returncode))
                            eprint(exc.output)
                            print_result(code_root_folder, cmake_result, make_result, app_result)
                            total_errors += 1
                        total_tests += 1
    print()
    print()
    if total_errors > 0:
        print('{errs} error(s) detected. {oks} test(s) of {totals} succeeded.'.format(errs=total_errors, oks=total_oks, totals=total_tests))
        sys.exit(1)
    else:
        print('Ran {totals} tests. No error detected. Have a nice day :)'.format(totals=total_tests))
        sys.exit(0)



if __name__ == "__main__":
    main()

