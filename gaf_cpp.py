#!/usr/bin/env python3
import os

from gaf_types import StandardType, Struct, get_unique_types, File, Member, OutputOptions, CppEnumStyle, Enum, \
    CppJsonReturn


class Out:
    def __init__(self):
        self.header = []
        self.source = []

    def add_source(self, line: str):
        self.source.append(line)

    def add_header(self, line: str):
        self.header.append(line)


class VarValue:
    def __init__(self, variable: str, value: str):
        self.variable = variable
        self.value = value


def json_return_value(opt: OutputOptions) -> str:
    if opt.json_return == CppJsonReturn.Char:
        return 'const char*'
    if opt.json_return == CppJsonReturn.Bool:
        return 'bool'
    if opt.json_return == CppJsonReturn.String:
        return 'std::string'
    return 'Unhandled_type_CppReturnValue'


def json_is_false(opt: OutputOptions) -> str:
    if opt.json_return == CppJsonReturn.Char:
        return '!=nullptr'
    if opt.json_return == CppJsonReturn.Bool:
        return '==false'
    if opt.json_return == CppJsonReturn.String:
        return '.empty() == false'
    return 'Unhandled_type_CppReturnValue'


def json_return_error(opt: OutputOptions, err: str, val: str) -> str:
    if opt.json_return == CppJsonReturn.Char:
        return 'return "{}";'.format(err)
    if opt.json_return == CppJsonReturn.Bool:
        return 'return false;'
    if opt.json_return == CppJsonReturn.String:
        return '{{ gaf_ss.str(""); gaf_ss << "{err}, path: " << gaf_path << ", value: " << GafToString({val}); return gaf_ss.str(); }}'.format(err=err, val=val)
    return 'return Unhandled_err_CppReturnValue;'


def json_return_ok(opt: OutputOptions) -> str:
    if opt.json_return == CppJsonReturn.Char:
        return 'nullptr'
    if opt.json_return == CppJsonReturn.Bool:
        return 'true'
    if opt.json_return == CppJsonReturn.String:
        return '""'
    return 'Unhandled_OK_CppReturnValue'


def get_cpp_parse_from_rapidjson_helper_int(opt: OutputOptions, sources: Out, t: StandardType, member: str, indent: str, name: str, json: str) -> VarValue:
    line = '{i}if({j}.IsInt64()==false) {rti}\n' \
           '{i}auto gafv = {j}.GetInt64();\n' \
           '{i}if(gafv < std::numeric_limits<{t}>::min()) {rtl}\n' \
           '{i}if(gafv > std::numeric_limits<{t}>::max()) {rth}\n'\
        .format(m=member, i=indent, t=t.get_cpp_type(), n=name, j=json,
                rti=json_return_error(opt, "read value for {n} was not a integer".format(m=member, i=indent, t=t.get_cpp_type(), n=name, j=json), json),
                rtl=json_return_error(opt, "read value for {n} was to low".format(m=member, i=indent, t=t.get_cpp_type(), n=name, j=json), 'gafv'),
                rth=json_return_error(opt, "read value for {n} was to high".format(m=member, i=indent, t=t.get_cpp_type(), n=name, j=json), 'gafv')
                )
    sources.add_source(line)
    var = 'c->{m}'.format(m=member)
    val = 'static_cast<{t}>(gafv)'.format(t=t.get_cpp_type())
    return VarValue(variable=var, value=val)


def get_cpp_parse_from_rapidjson_helper_float(opt: OutputOptions, sources: Out, member: str, indent: str, name: str, json: str) -> VarValue:
    line = '{i}if({j}.IsNumber()==false) {err} \n'\
        .format(m=member, i=indent, n=name, j=json,
                err=json_return_error(opt, "read value for {n} was not a number".format(m=member, i=indent, n=name, j=json), json))
    sources.add_source(line)
    var = 'c->{m}'.format(m=member)
    val = '{}.GetDouble()'.format(json)
    return VarValue(variable=var, value=val)


def get_cpp_parse_from_rapidjson_base(opt: OutputOptions, sources: Out, t: StandardType, member: str, indent: str, name: str, json: str) -> VarValue:
    # todo: verify that all int parsing ranges are correct
    if t == StandardType.int8:
        return get_cpp_parse_from_rapidjson_helper_int(opt, sources, t, member, indent, name, json)
    elif t == StandardType.int16:
        return get_cpp_parse_from_rapidjson_helper_int(opt, sources, t, member, indent, name, json)
    elif t == StandardType.int32:
        return get_cpp_parse_from_rapidjson_helper_int(opt, sources, t, member, indent, name, json)
    elif t == StandardType.int64:
        line = '{i}if({j}.IsInt64()==false) {err} \n'\
            .format(m=member, i=indent, t=t, n=name, j=json,
                    err=json_return_error(opt, "read value for {n} was not a integer".format(m=member, i=indent, t=t, n=name, j=json), json))
        var = 'c->{m}'.format(m=member)
        val = '{}.GetInt64()'.format(json)
        sources.add_source(line)
        return VarValue(variable=var, value=val)
    elif t == StandardType.uint8:
        return get_cpp_parse_from_rapidjson_helper_int(opt, sources, t, member, indent, name, json)
    elif t == StandardType.uint16:
        return get_cpp_parse_from_rapidjson_helper_int(opt, sources, t, member, indent, name, json)
    elif t == StandardType.uint32:
        return get_cpp_parse_from_rapidjson_helper_int(opt, sources, t, member, indent, name, json)
    elif t == StandardType.uint64:
        return get_cpp_parse_from_rapidjson_helper_int(opt, sources, t, member, indent, name, json)
    elif t == StandardType.float:
        return get_cpp_parse_from_rapidjson_helper_float(opt, sources, member, indent, name, json)
    elif t == StandardType.double:
        return get_cpp_parse_from_rapidjson_helper_float(opt, sources, member, indent, name, json)
    elif t == StandardType.byte:
        return get_cpp_parse_from_rapidjson_helper_int(opt, sources, t, member, indent, name, json)
    elif t == StandardType.bool:
        line = '{i}if({j}.IsBool()==false) {err} \n'\
            .format(m=member, i=indent, t=t, n=name, j=json,
                    err=json_return_error(opt, "read value for {n} was not a bool".format(m=member, i=indent, t=t, n=name, j=json), json))
        var = 'c->{m}'.format(m=member)
        val = '{}.GetBool()'.format(json)
        sources.add_source(line)
        return VarValue(variable=var, value=val)
    elif t == StandardType.string:
        line = '{i}if({j}.IsString()==false) {err} \n'\
            .format(m=member, i=indent, t=t, n=name, j=json,
                    err=json_return_error(opt, "read value for {n} was not a string".format(m=member, i=indent, t=t, n=name, j=json), json))
        var = 'c->{m}'.format(m=member)
        val = '{}.GetString()'.format(json)
        sources.add_source(line)
        return VarValue(variable=var, value=val)
    else:
        print('BUG: No type specified')
        return VarValue(variable=member, value='bug_unhandled_std_type')


def get_cpp_parse_from_rapidjson(opt: OutputOptions, sources: Out, t: StandardType, member: str, indent: str, name: str, member_type: Member) -> None:
    if member_type.is_dynamic_array:
        line = '{i}const rapidjson::Value& arr = iter->value;\n' \
               '{i}if(!arr.IsArray()) {err}\n' \
               '{i}for (rapidjson::SizeType i=0; i<arr.Size(); i++)\n' \
               '{i}{{\n' \
            .format(i=indent, name=name,
                    err=json_return_error(opt, "tried to read {name} but value was not a array".format(i=indent, name=name), 'arr'))
        sources.add_source(line)
        vv = get_cpp_parse_from_rapidjson_base(opt, sources, t, member, indent + '  ', name, 'arr[i]')
        sources.add_source('{i}  {var}.push_back({val});\n'.format(i=indent, var=vv.variable, val=vv.value))
        sources.add_source('{}}}\n'.format(indent))
    else:
        vv = get_cpp_parse_from_rapidjson_base(opt, sources, t, member, indent, name, 'iter->value')
        if member_type.is_optional:
            sources.add_source('{i}{var} = std::make_shared<{cpptype}>({val});\n'.format(i=indent, var=vv.variable, val=vv.value, cpptype=t.get_cpp_type()))
        else:
            sources.add_source('{i}{var} = {val};\n'.format(i=indent, var=vv.variable, val=vv.value))


def write_json_member(opt: OutputOptions, m: Member, sources: Out, indent):
    if m.typename.standard_type != StandardType.INVALID:
        get_cpp_parse_from_rapidjson(opt, sources, m.typename.standard_type, m.name, indent, m.name, m)
    else:
        if m.is_dynamic_array:
            lines = []
            lines.append('{i}const rapidjson::Value& arr = iter->value;\n')
            lines.append('{i}if(!arr.IsArray()) {err}\n')
            lines.append('{i}for (rapidjson::SizeType i=0; i<arr.Size(); i++)\n')
            lines.append('{i}{{\n')
            lines.append('{i}  {type} temp;\n')
            if opt.json_return == CppJsonReturn.String:
                lines.append('{i}  gaf_ss.str("");\n')
                lines.append('{i}  gaf_ss << gaf_path << ".{name}[" << i << "]";\n')
                lines.append('{i}  {rv} r = ReadFromJsonValue(&temp,arr[i], gaf_ss.str());\n')
            else:
                lines.append('{i}  {rv} r = ReadFromJsonValue(&temp,arr[i]);\n')
            lines.append('{i}  if(r{false}) {{ return r; }}\n')
            lines.append('{i}  c->{name}.push_back(temp);\n')
            lines.append('{i}}}\n')
            for line in lines:
                sources.add_source(line.format(i=indent, name=m.name, type=m.typename.name,
                                               err=json_return_error(opt, "tried to read {name} but value was not a array".format(i=indent, name=m.name, type=m.typename.name), 'arr'),
                                               false=json_is_false(opt),
                                               rv=json_return_value(opt)))
        elif m.is_optional:
            lines = []
            lines.append('{i}c->{name} = std::make_shared<{type}>();\n')
            if opt.json_return == CppJsonReturn.String:
                lines.append('{i}gaf_ss.str("");\n')
                lines.append('{i}gaf_ss << gaf_path << ".{name}";\n')
                lines.append('{i}{rv} r = ReadFromJsonValue(c->{name}.get(),iter->value, gaf_ss.str());\n')
            else:
                lines.append('{i}{rv} r = ReadFromJsonValue(c->{name}.get(),iter->value);\n')
            lines.append('{i}if(r{false})\n')
            lines.append('{i}{{\n')
            lines.append('{i}  c->{name}.reset();\n')
            lines.append('{i}  return r;\n')
            lines.append('{i}}}\n')
            for line in lines:
                sources.add_source(line.format(i=indent, name=m.name, type=m.typename.name,false=json_is_false(opt), rv=json_return_value(opt)))
        else:
            lines = []
            if opt.json_return == CppJsonReturn.String:
                lines.append('{i}gaf_ss.str("");\n')
                lines.append('{i}gaf_ss << gaf_path << ".{name}";\n')
                lines.append('{i}{rv} r = ReadFromJsonValue(&c->{name},iter->value, gaf_ss.str());\n')
            else:
                lines.append('{i}{rv} r = ReadFromJsonValue(&c->{name},iter->value);\n')
            lines.append('{i}if(r{false})\n')
            lines.append('{i}{{\n')
            lines.append('{i}  return r;\n')
            lines.append('{i}}}\n')
            for line in lines:
                sources.add_source(line.format(i=indent, name=m.name, false=json_is_false(opt), rv=json_return_value(opt)))


def write_json_source_for_cpp(write_json: bool, sources: Out, s: Struct, opt: OutputOptions):
    if write_json:
        if opt.json_return == CppJsonReturn.String:
            sources.add_source('{rv} ReadFromJsonValue({struct}* c, const rapidjson::Value& value, const std::string& gaf_path) {{\n'.format(struct=s.name, rv=json_return_value(opt)))
            sources.add_source('  std::stringstream gaf_ss;\n')
        else:
            sources.add_source('{rv} ReadFromJsonValue({struct}* c, const rapidjson::Value& value) {{\n'.format(struct=s.name, rv=json_return_value(opt)))
        sources.add_source('  if(!value.IsObject()) {error}\n'.format(error=json_return_error(opt, "tried to read {} but value was not a object".format(s.name), 'value')))
        sources.add_source('  rapidjson::Value::ConstMemberIterator iter;\n')
        for m in s.members:
            sources.add_source('  iter = value.FindMember("{n}");\n'.format(n=m.name))
            sources.add_source('  if(iter != value.MemberEnd()) {\n')
            write_json_member(opt, m, sources, '    ')
            sources.add_source('  }\n')
            if m.missing_is_fail or m.is_optional:
                sources.add_source('  else {\n')
                if m.is_optional:
                    sources.add_source('    c->{}.reset();\n'.format(m.name))
                else:
                    sources.add_source('    {error}\n'.format(error=json_return_error(opt, "missing {} in json object".format(m.name), 'value')))
                sources.add_source('  }\n')
        sources.add_source('  return {};\n'.format(json_return_ok(opt)))
        sources.add_source('}\n')
        sources.add_source('\n')


def write_single_imgui_member_to_source(name: str, var: str, t: StandardType, sources: Out, indent: str):
    if t == StandardType.int8:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.int16:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.int32:
        sources.add_source('{i}ImGui::InputInt("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.int64:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.uint8:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.uint16:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.uint32:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.uint64:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.float:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.double:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.byte:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.bool:
        sources.add_source('{i}ImGui::Checkbox("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.string:
        sources.add_source('{i}ImGui::Edit("{name}", &c->{var});\n'.format(name=name, var=var, i=indent))
    else:
        sources.add_source('{i}// todo: Unhandled type: {name} / {var}\n'.format(name=name, var=var, i=indent))


def write_single_member_to_source(m: Member, sources: Out):
    if not m.is_dynamic_array:
        write_single_imgui_member_to_source(m.name, m.name, m.typename.standard_type, sources, '  ')
    else:
        sources.add_source('  if(ImGui::TreeNode("{name}"))\n'.format(name=m.name, var=m.name))
        sources.add_source('  {\n')
        sources.add_source('    for(std::size_t i=0; i<c->{var}.size(); i+= 1)\n'.format(var=m.name))
        sources.add_source('    {\n')
        sources.add_source('      ImGui::PushID(i);\n')
        write_single_imgui_member_to_source('', '{}[i]'.format(m.name), m.typename.standard_type, sources, '      ')
        sources.add_source('      ImGui::SameLine();\n')
        sources.add_source('      ImGui::Button("Delete##{}");\n'.format(m.name))
        sources.add_source('      ImGui::PopID();\n')
        sources.add_source('    }\n')
        sources.add_source('    ImGui::Button("Add");\n')
        sources.add_source('    ImGui::TreePop();\n')
        sources.add_source('  }\n')


def write_imgui_source_for_cpp(write_imgui: bool, sources: Out, s: Struct, opt: OutputOptions):
    if write_imgui:
        sources.add_source('void RunImgui({name}* c)\n'.format(name=s.name))
        sources.add_source('{\n')
        for m in s.members:
            write_single_member_to_source(m, sources)
        sources.add_source('}\n')
        sources.add_source('\n')


def write_member_variables_for_cpp(sources: Out, s: Struct, opt: OutputOptions):
    # sources.add_header(' public:\n')
    for m in s.members:

        # m.typename.is_enum
        type_name = '{}::Type'.format(m.typename.name)\
            if m.typename.is_enum and opt.enum_style == CppEnumStyle.NamespaceEnum else m.typename.name
        if m.is_optional:
            sources.add_header('  std::shared_ptr<{tn}> {n};\n'.format(n=m.name, tn=type_name))
        elif m.is_dynamic_array:
            sources.add_header('  std::vector<{tn}> {n};\n'.format(n=m.name, tn=type_name))
        else:
            sources.add_header('  {tn} {n};\n'.format(n=m.name, tn=type_name))
    sources.add_header('}}; // class {}\n'.format(s.name))


def write_default_constructor_for_cpp(s: Struct, sources: Out, opt: OutputOptions):
    common_members = [x for x in s.members if x.defaultvalue is not None]
    if len(common_members) > 0:
        sources.add_header('  {}();\n'.format(s.name))
        sources.add_header('\n')
        sources.add_source('{n}::{n}()\n'.format(n=s.name))
        sep = ':'
        for m in common_members:
            default_value = m.defaultvalue
            if m.typename.is_enum:
                default_value = '{t}{colon}{v}'.format(t=m.typename.name, v=m.defaultvalue,
                                                       colon='_' if opt.enum_style == CppEnumStyle.PrefixEnum else '::')
            sources.add_source('  {s} {n}({d})\n'.format(s=sep, n=m.name, d=default_value))
            sep = ','
        sources.add_source('{}\n')
        sources.add_source('\n')


def iterate_enum(e: Enum, sources: Out, prefix_prop: bool=False):
    prefix = '{}_'.format(e.name) if prefix_prop else ''
    for i, v in enumerate(e.values, start=1):
        last = i == len(e.values)
        comma = '' if last else ','
        sources.add_header('  {p}{v}{c}\n'.format(p=prefix, v=v, c=comma))


def add_enum_json_function(e: Enum, sources: Out, opt: OutputOptions, prefix_prop: bool=False, type_enum: bool=False):
    enum_type = '{}::Type'.format(e.name) if type_enum else e.name
    value_prefix = '{}_'.format(e.name) if prefix_prop else '{}::'.format(e.name)
    arg = ', const std::string& gaf_path' if opt.json_return == CppJsonReturn.String else ''
    sources.add_header('{rv} ReadFromJsonValue({t}* c, const rapidjson::Value& value{a});\n'.format(t=enum_type, rv=json_return_value(opt), a=arg))
    sources.add_source('{rv} ReadFromJsonValue({t}* c, const rapidjson::Value& value{a})\n'.format(t=enum_type, rv=json_return_value(opt), a=arg))
    sources.add_source('{{\n')
    if opt.json_return == CppJsonReturn.String:
        sources.add_source('  std::stringstream gaf_ss;\n')
    sources.add_source('  if(value.IsString()==false) {err};\n'.format(err=json_return_error(opt, "read value for {e} was not a string".format(e=e.name), 'value')))
    for v in e.values:
        sources.add_source('  if(strcmp(value.GetString(), "{v}")==0) {{ *c = {p}{v}; return {ok};}}\n'.format(p=value_prefix, v=v, ok=json_return_ok(opt)))
    sources.add_source('  {}\n'.format(json_return_error(opt, "read string for {e} was not valid".format(e=e.name), 'value')))
    sources.add_source('}}\n')
    sources.add_source('\n')


def generate_cpp(f: File, sources: Out, name: str, opt: OutputOptions):
    headerguard = 'GENERATED_' + name.upper()

    # get all standard types used for typedefing later on...
    unique_types = get_unique_types(f)
    default_types = [t for t in unique_types
                     if t.standard_type.get_cpp_type() != '' and t.name != t.get_cpp_type()
                     ]

    json_string = opt.write_json and opt.json_return == CppJsonReturn.String
    has_string = StandardType.string in [t.standard_type for t in unique_types]
    has_dynamic_arrays = any(m for s in f.structs for m in s.members if m.is_dynamic_array)
    has_optional = any(m for s in f.structs for m in s.members if m.is_optional)

    sources.add_header('#ifndef {}_H\n'.format(headerguard))
    sources.add_header('#define {}_H\n'.format(headerguard))
    sources.add_header('\n')

    added_include = False
    if len(default_types) > 0:
        added_include = True
        sources.add_header('#include <cstdint>\n')
    if has_string or json_string:
        added_include = True
        sources.add_header('#include <string>\n')
    if json_string and opt.header_only:
        added_include = True
        sources.add_header('#include <sstream>\n')

    if len(f.enums) > 0 and opt.write_json:
        if opt.header_only:
            sources.add_header('#include <cstring>\n')
        else:
            sources.add_source('#include <cstring>\n')
    if has_dynamic_arrays:
        added_include = True
        sources.add_header('#include <vector>\n')
    if has_optional:
        added_include = True
        sources.add_header('#include <memory>\n')
    if added_include:
        sources.add_header('\n')

    if opt.write_json and opt.header_only:
        sources.add_header('#include <limits>\n')
        sources.add_header('#include "rapidjson/document.h"\n')
        sources.add_header('\n')

    if opt.write_json and not opt.header_only:
        # todo: forward decalre rapidjson::Value
        sources.add_header('#include "rapidjson/document.h"\n')
        sources.add_header('\n')

    if f.package_name != '':
        sources.add_header('namespace {} {{\n'.format(f.package_name))
        sources.add_header('\n')

    if len(default_types) > 0:
        for t in default_types:
            sources.add_header('typedef {ct} {t};\n'.format(t=t.name, ct=t.get_cpp_type()))
        sources.add_header('\n')

    if len(f.typedefs) > 0:
        for s in f.typedefs:
            sources.add_header('class {};\n'.format(s.name))
        sources.add_header('\n')

    for e in f.enums:
        if opt.enum_style == CppEnumStyle.EnumClass:
            sources.add_header('enum class {} {{\n'.format(e.name))
            iterate_enum(e, sources)
            sources.add_header('}}; // enum {}\n'.format(e.name))
            if opt.write_json:
                add_enum_json_function(e, sources, opt)
        elif opt.enum_style == CppEnumStyle.NamespaceEnum:
            sources.add_header('namespace {} {{ enum Type {{\n'.format(e.name))
            iterate_enum(e, sources)
            sources.add_header('}}; }} // namespace enum {}\n'.format(e.name))
            if opt.write_json:
                add_enum_json_function(e, sources, opt, type_enum=True)
        elif opt.enum_style == CppEnumStyle.PrefixEnum:
            sources.add_header('enum {} {{\n'.format(e.name))
            iterate_enum(e, sources, True)
            sources.add_header('}}; // enum {}\n'.format(e.name))
            if opt.write_json:
                add_enum_json_function(e, sources, opt, prefix_prop=True)
        else:
            sources.add_header('code generation failed, unhandled enum style {}'.format(opt.enum_style))

        sources.add_header('\n')

    for s in f.structs_defined:
        sources.add_header('class {} {{\n'.format(s.name))
        sources.add_header(' public:\n')
        write_default_constructor_for_cpp(s, sources, opt)

        write_json_source_for_cpp(opt.write_json, sources, s, opt)
        write_imgui_source_for_cpp(opt.write_imgui, sources, s, opt)

        write_member_variables_for_cpp(sources, s, opt)

        if opt.write_json:
            sources.add_header('\n')
            arg = ', const std::string& gaf_path' if opt.json_return == CppJsonReturn.String else ''
            sources.add_header('{rv} ReadFromJsonValue({name}* c, const rapidjson::Value& value{a});\n'.format(name=s.name, rv=json_return_value(opt), a=arg))
        if opt.write_imgui:
            sources.add_header('\n')
            sources.add_header('void RunImgui({name}* c);\n'.format(name=s.name))
        sources.add_header('\n')

    if opt.json_return == CppJsonReturn.String and opt.write_json:
        sources.add_header('std::string GafToString(const rapidjson::Value& val);\n')
        sources.add_source('std::string GafToString(const rapidjson::Value& val)\n')
        sources.add_source('{\n')
        sources.add_source('  if(val.IsNull()) { return "Null"; };\n')
        sources.add_source('  if(val.IsFalse()) { return "False"; };\n')
        sources.add_source('  if(val.IsTrue()) { return "True"; };\n')
        sources.add_source('  if(val.IsObject()) { return "Object"; };\n')
        sources.add_source('  if(val.IsArray()) { return "Array"; };\n')
        sources.add_source('  if(val.IsUint64()) { std::stringstream ss; ss << "uint of " << val.GetUint64(); return ss.str(); };\n')
        sources.add_source('  if(val.IsInt64()) { std::stringstream ss; ss << "int of " << val.GetInt64(); return ss.str(); };\n')
        sources.add_source('  if(val.IsDouble()) { std::stringstream ss; ss << "double of " << val.GetDouble(); return ss.str(); };\n')
        sources.add_source('  if(val.IsString()) { std::stringstream ss; ss << "string of " << val.GetString(); return ss.str(); };\n')
        sources.add_source('  return "<unknown>";\n')
        sources.add_source('}\n')
        sources.add_source('\n')

        # todo: remove this horrible function
        sources.add_header('std::string GafToString(int64_t val);\n')
        sources.add_source('std::string GafToString(int64_t val)\n')
        sources.add_source('{\n')
        sources.add_source('  std::stringstream ss;\n')
        sources.add_source('  ss << val;\n')
        sources.add_source('  return ss.str();\n')
        sources.add_source('}\n')
        sources.add_source('\n')

        sources.add_header('\n')

    if opt.header_only:
        sources.add_header('#ifdef {}_IMPLEMENTATION\n'.format(headerguard))
        sources.add_header('\n')
        for s in sources.source:
            sources.add_header(s)
        sources.add_header('#endif // {}_IMPLEMENTATION\n'.format(headerguard))

    if f.package_name != '':
        sources.add_header('}} // namespace {}\n'.format(f.package_name))
        sources.add_header('\n')
    sources.add_header('\n')
    sources.add_header('#endif  // {}_H\n'.format(headerguard))


def write_cpp(f: File, opt: OutputOptions, out_dir: str, name: str):
    sources = Out()
    generate_cpp(f, sources, name, opt)

    with open(os.path.join(out_dir, opt.prefix + name + '.h'), 'w', encoding='utf-8') as out:
        for s in sources.header:
            out.write(s)

    if not opt.header_only:
        with open(os.path.join(out_dir, opt.prefix + name + '.cc'), 'w', encoding='utf-8') as out:
            out.write('#include "{}.h"\n'.format(opt.prefix + name))
            out.write('\n')
            out.write('#include <limits>\n')
            if opt.write_json:
                if opt.json_return == CppJsonReturn.String:
                    out.write('#include <sstream>\n')
                out.write('\n')
                out.write('#include "rapidjson/document.h"\n')
            if opt.write_imgui:
                out.write('#include {}\n'.format(opt.imgui_header))
                out.write('\n')
            out.write('\n')

            if f.package_name != '':
                out.write('namespace {} {{\n'.format(f.package_name))
                out.write('\n')

            for s in sources.source:
                out.write(s)

            if f.package_name != '':
                out.write('}} // namespace {}\n'.format(f.package_name))
                out.write('\n')
