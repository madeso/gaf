#!/usr/bin/env python3

import enum
from enum import Enum
import typing


@enum.unique
class StandardType(Enum):
    int8 = object()
    int16 = object()
    int32 = object()
    int64 = object()
    float = object()
    double = object()
    byte = object()
    string = object()
    INVALID = object()

    def get_cpp_type(self) -> str:
        if self == StandardType.int8:
            return 'int8_t'
        if self == StandardType.int16:
            return 'int16_t'
        if self == StandardType.int32:
            return 'int32_t'
        if self == StandardType.int64:
            return 'int64_t'
        if self == StandardType.float:
            return 'float'
        if self == StandardType.double:
            return 'double'
        if self == StandardType.byte:
            return 'char'
        if self == StandardType.string:
            return 'std::string'
        return ''


class Type:
    def __init__(self, standard_type: StandardType, name: str, pass_as_value: bool,
                 default_value: typing.Optional[str]=None):
        self.name = name
        self.standard_type = standard_type
        self.pass_as_value = pass_as_value
        self.default_value = default_value

    def get_cpp_type(self) -> str:
        if self.standard_type == StandardType.INVALID:
            return self.name
        else:
            return self.standard_type.get_cpp_type()


class TypeList:
    def __init__(self):
        self.types = dict()

    def add_type(self, t: Type):
        # todo: handle duplicate defined types
        self.types[t.name] = t

    def add_default_types(self):
        self.add_type(Type(StandardType.int8, 'int8', True, '0'))
        self.add_type(Type(StandardType.int16, 'int16', True, '0'))
        self.add_type(Type(StandardType.int32, 'int32', True, '0'))
        self.add_type(Type(StandardType.int64, 'int64', True, '0'))
        self.add_type(Type(StandardType.float, 'float', True, '0.0f'))
        self.add_type(Type(StandardType.double, 'double', True, '0.0'))
        self.add_type(Type(StandardType.byte, 'byte', True, '0'))
        self.add_type(Type(StandardType.string, 'string', False))

    def is_valid_type(self, name: str) -> bool:
        return name in self.types

    def get_type(self, name: str) -> Type:
        return self.types[name]


def is_default_type(tn: str) -> bool:
    tl = TypeList()
    tl.add_default_types()
    return tl.is_valid_type(tn)


class Member:
    def __init__(self, name: str, typename: Type):
        self.name = name
        self.typename = typename
        self.defaultvalue = typename.default_value

    def __str__(self):
        if self.defaultvalue is None:
            return '{tn} {n};'.format(n=self.name, tn=self.typename.name)
        else:
            return '{tn} {n} = {dv};'.format(n=self.name, tn=self.typename.name, dv=self.defaultvalue)


def empty_member_list() -> typing.List[Member]:
    return []


class Struct:
    def __init__(self, name: str):
        self.name = name
        self.members = empty_member_list()

    def add_member(self, member: Member):
        self.members.append(member)

    def __str__(self):
        return 'struct {n} {{\n{mem}\n}}'.format(n=self.name, mem='\n'.join(['  ' + str(x) for x in self.members]))


def empty_struct_list() -> typing.List[Struct]:
    return []


class Constant:
    def __init__(self, n: str, t: Type, v: str):
        self.name = n
        self.type = t
        self.value = v


def empty_constant_list() -> typing.List[Constant]:
    return []


class File:
    def __init__(self):
        self.structs = empty_struct_list()
        self.constants = empty_constant_list()
        self.package_name = ''

    def __str__(self):
        package_name = ''
        if len(self.package_name) > 0:
            package_name = 'package {};\n'.format(self.package_name)
        return package_name + '\n'.join([str(x) for x in self.structs])

    def add_constant(self, n: str, t: Type, v: str):
        self.constants.append(Constant(n, t, v))

    def find_constant(self, name: str, ty: typing.Optional[Type]) -> typing.Optional[Constant]:
        for c in self.constants:
            if ty is None:
                if c.name == name:
                    return c
            elif c.name == name and c.type == ty:
                return c
        return None


def merge(iters):
    for it in iters:
        yield from it


def get_unique_types(f: File) -> typing.Set[Type]:
    return set(m.typename for m in merge(s.members for s in f.structs))