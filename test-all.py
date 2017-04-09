#!/usr/bin/env python3
# GAF test, please ignore

import os
import subprocess
import shutil

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

    code_examples = [Code('onestruct')]

    for code in code_examples:
        code_root_folder = os.path.join(build_folder, code.name)
        remove_folder(code_root_folder)
        ensure_folder_exist(code_root_folder)
        with open(os.path.join(code_root_folder, 'CMakeLists.txt'), 'w') as cmake_file:
            test_cpp_folder = os.path.join(root_folder, 'test-cpp')
            cmake_file.write('''
            cmake_minimum_required(VERSION 3.1)
            project(gaf)
            set (CMAKE_CXX_STANDARD 11)
            include_directories(SYSTEM {external}/catch)
            include({root}/gaf.cmake)
            SET(Gaf_CUSTOM_NAME mygaf)
            GAF_GENERATE_CPP(GAF_SOURCES GAF_HEADERS {root}/examples/{gaf})
            include_directories(${{CMAKE_CURRENT_BINARY_DIR}})
            add_executable(app {cpp} ${{GAF_SOURCES}} ${{GAF_HEADERS}})
            '''.format(
                gaf=code.gaf,
                cpp= os.path.join(test_cpp_folder, code.test),
                external=os.path.join(test_cpp_folder, 'external'),
                root=root_folder
            ))
        code_build_folder = os.path.join(code_root_folder, 'build')
        ensure_folder_exist(code_build_folder)
        print('running cmake')
        cmake_result = subprocess.check_call(['cmake', '..'], cwd=code_build_folder)
        print('running make')
        make_result = subprocess.check_call(['make'], cwd=code_build_folder)
        print('running code')
        code_run_folder = os.path.join(code_build_folder, 'run')
        ensure_folder_exist(code_run_folder)
        app_result = subprocess.check_output(['../app'], cwd=code_run_folder)
        print(app_result)


if __name__ == "__main__":
    main()

