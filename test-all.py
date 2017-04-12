#!/usr/bin/env python3
# GAF test, please ignore

import os
import sys
import subprocess
import shutil


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def ensure_folder_exist(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)


class Code:
    def __init__(self, name, gaf=None, test=None):
        self.name = name
        self.gaf = gaf if gaf is not None else name+'.c'
        self.test = test if test is not None else name+'.cc'


def remove_folder(d):
    if os.path.exists(d):
        shutil.rmtree(d)


def main():
    root_folder = os.getcwd()
    print('root', root_folder)

    build_folder = os.path.join(root_folder, 'build')

    ensure_folder_exist(build_folder)

    code_examples = [
        Code('comments1', test = 'onestruct.cc'),
        Code('comments2', test = 'onestruct.cc'),
        Code('comments3', test = 'onestruct.cc'),
        Code('onestruct'),
        Code('package'),
        Code('twostructs')
    ]

    errors = 0

    for code in code_examples:
        for json_test in [True, False]:
            codename = code.name
            if json_test:
                codename += '_json'

            print()
            print('----------------------------------------------------------------')
            print('--- {}'.format(codename))
            print('----------------------------------------------------------------')
            code_root_folder = os.path.join(build_folder, code.name)

            remove_folder(code_root_folder)
            ensure_folder_exist(code_root_folder)
            with open(os.path.join(code_root_folder, 'cmdlineargs.txt'), 'w') as cmd:
                if json_test:
                    cmd.write('--include-json\n')

            with open(os.path.join(code_root_folder, 'CMakeLists.txt'), 'w') as cmake_file:
                test_cpp_folder = os.path.join(root_folder, 'test-cpp')
                cmake_file.write('''
                cmake_minimum_required(VERSION 3.1)
                project(gaf)
                set (CMAKE_CXX_STANDARD 11)
                include_directories(SYSTEM {external}/catch)
                include_directories(SYSTEM {external}/rapidjson-1.1.0/include)
                include({root}/gaf.cmake)
                SET(Gaf_CUSTOM_NAME mygaf)
                SET(Gaf_CUSTOM_ARGUMENTS_FROM_FILE {coderoot}cmdlineargs.txt)
                GAF_GENERATE_CPP(GAF_SOURCES GAF_HEADERS {root}/examples/{gaf})
                include_directories(${{CMAKE_CURRENT_BINARY_DIR}})
                add_executable(app {cpp} ${{GAF_SOURCES}} ${{GAF_HEADERS}})
                '''.format(
                    gaf=code.gaf,
                    cpp= os.path.join(test_cpp_folder, code.test),
                    external=os.path.join(test_cpp_folder, 'external'),
                    root=root_folder,
                    coderoot=code_root_folder
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
            except subprocess.CalledProcessError as exc:
                eprint('Status: FAIL')
                eprint('Error code: {}'.format(exc.returncode))
                eprint(exc.output)
                if len(cmake_result)>0:
                    print('CMake result:')
                    print(cmake_result)
                    print()
                if len(make_result)>0:
                    print('Make result:')
                    print(make_result)
                    print()
                if len(app_result) > 0:
                    print('app result:')
                    print(app_result)
                    print()
                errors += 1
    print()
    print()
    if errors > 0:
        print('{} error(s) detected'.format(errors))
        sys.exit(1)
    else:
        print('No error detected. Have a nice day :)')
        sys.exit(0)



if __name__ == "__main__":
    main()

