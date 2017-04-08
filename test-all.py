#!/usr/bin/env python3
# GAF test, please ignore

import os
import subprocess
import shutil

def ensure_dir(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)


class Code:
    def __init__(self, name, gaf=None, test=None):
        self.name = name
        self.gaf = gaf if gaf is not None else name+'.c'
        self.test = test if test is not None else name+'.cc'


def main():
    root = os.getcwd()
    print('root', root)

    build = os.path.join(root, 'build')
    print('build', build)

    ensure_dir(build)
    # remove everything in build

    examples = [Code('onestruct')]

    for c in examples:
        d = os.path.join(build, c.name)
        print('  specific dir', d)
        if os.path.exists(d):
            shutil.rmtree(d)
        ensure_dir(d)
        with open(os.path.join(d, 'CMakeLists.txt'), 'w') as f:
            f.write('''
            cmake_minimum_required(VERSION 2.8.9)
            project(gaf)
            add_executable(gaf {cpp})
            '''.format(gaf=c.gaf, cpp= os.path.join(root, 'test-cpp', c.test) ))
        b = os.path.join(d, 'build')
        ensure_dir(b)
        called = subprocess.call(['cmake', '..'], cwd=b)
        print("called", called)
        if called == 0:
            mr = subprocess.call(['make'], cwd=b)
            print("mr", mr)
            if mr == 0:
                print(b)
                xx = subprocess.call(['./gaf'], cwd=b)
                print(xx)



if __name__ == "__main__":
    main()

