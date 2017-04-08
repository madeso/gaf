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
    print('build', build_folder)

    ensure_folder_exist(build_folder)
    # remove everything in build

    code_examples = [Code('onestruct')]

    for code in code_examples:
        code_root_folder = os.path.join(build_folder, code.name)
        print('  specific dir', code_root_folder)
        remove_folder(code_root_folder)
        ensure_folder_exist(code_root_folder)
        with open(os.path.join(code_root_folder, 'CMakeLists.txt'), 'w') as cmake_file:
            cmake_file.write('''
            cmake_minimum_required(VERSION 2.8.9)
            project(gaf)
            add_executable(app {cpp})
            '''.format(gaf=code.gaf, cpp= os.path.join(root_folder, 'test-cpp', code.test) ))
        code_build_folder = os.path.join(code_root_folder, 'build')
        ensure_folder_exist(code_build_folder)
        cmake_result = subprocess.call(['cmake', '..'], cwd=code_build_folder)
        print("called", cmake_result)
        if cmake_result == 0:
            make_result = subprocess.call(['make'], cwd=code_build_folder)
            print("mr", make_result)
            if make_result == 0:
                print(code_build_folder)
                app_result = subprocess.call(['./app'], cwd=code_build_folder)
                print(app_result)



if __name__ == "__main__":
    main()

