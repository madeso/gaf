#!/usr/bin/env python3
import os
import typing

from gaf_types import StandardType, Struct, get_unique_types, File, Member, Enum, TypeList, Plugin


class ImguiOptions:
    def __init__(self, imgui_add, imgui_remove):
        self.imgui_add = imgui_add
        self.imgui_remove = imgui_remove


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


# todo(Gustav): remove this
def json_return_value() -> str:
    return 'std::string'


# todo(Gustav): remove this
def json_is_false() -> str:
    return '.empty() == false'


def json_return_error(err: str, val: str) -> str:
    return '{{ gaf_ss.str(""); gaf_ss << "{err}, path: " << gaf_path << ", value: " << GafToString({val}); return gaf_ss.str(); }}'.format(err=err, val=val)


def json_return_ok() -> str:
    return '""'


def get_cpp_parse_from_rapidjson_helper_int(sources: Out, t: StandardType, member: str, indent: str, name: str, json: str) -> VarValue:
    line = '{i}if({j}.IsInt64()==false) {rti}\n' \
           '{i}auto gafv = {j}.GetInt64();\n' \
           '{i}if(gafv < std::numeric_limits<{t}>::min()) {rtl}\n' \
           '{i}if(gafv > std::numeric_limits<{t}>::max()) {rth}\n'\
        .format(m=member, i=indent, t=t.get_cpp_type(), n=name, j=json,
                rti=json_return_error("read value for {n} was not a integer".format(m=member, i=indent, t=t.get_cpp_type(), n=name, j=json), json),
                rtl=json_return_error("read value for {n} was to low".format(m=member, i=indent, t=t.get_cpp_type(), n=name, j=json), 'gafv'),
                rth=json_return_error("read value for {n} was to high".format(m=member, i=indent, t=t.get_cpp_type(), n=name, j=json), 'gafv')
                )
    sources.add_source(line)
    var = 'c->{m}'.format(m=member)
    val = 'static_cast<{t}>(gafv)'.format(t=t.get_cpp_type())
    return VarValue(variable=var, value=val)


def get_cpp_parse_from_rapidjson_helper_float(sources: Out, member: str, indent: str, name: str, json: str) -> VarValue:
    line = '{i}if({j}.IsNumber()==false) {err} \n'\
        .format(m=member, i=indent, n=name, j=json,
                err=json_return_error("read value for {n} was not a number".format(m=member, i=indent, n=name, j=json), json))
    sources.add_source(line)
    var = 'c->{m}'.format(m=member)
    val = '{}.GetDouble()'.format(json)
    return VarValue(variable=var, value=val)


def get_cpp_parse_from_rapidjson_base(sources: Out, t: StandardType, member: str, indent: str, name: str, json: str) -> VarValue:
    # todo: verify that all int parsing ranges are correct
    if t == StandardType.int8:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.int16:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.int32:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.int64:
        line = '{i}if({j}.IsInt64()==false) {err} \n'\
            .format(m=member, i=indent, t=t, n=name, j=json,
                    err=json_return_error("read value for {n} was not a integer".format(m=member, i=indent, t=t, n=name, j=json), json))
        var = 'c->{m}'.format(m=member)
        val = '{}.GetInt64()'.format(json)
        sources.add_source(line)
        return VarValue(variable=var, value=val)
    elif t == StandardType.uint8:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.uint16:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.uint32:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.uint64:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.float:
        return get_cpp_parse_from_rapidjson_helper_float(sources, member, indent, name, json)
    elif t == StandardType.double:
        return get_cpp_parse_from_rapidjson_helper_float(sources, member, indent, name, json)
    elif t == StandardType.byte:
        return get_cpp_parse_from_rapidjson_helper_int(sources, t, member, indent, name, json)
    elif t == StandardType.bool:
        line = '{i}if({j}.IsBool()==false) {err} \n'\
            .format(m=member, i=indent, t=t, n=name, j=json,
                    err=json_return_error("read value for {n} was not a bool".format(m=member, i=indent, t=t, n=name, j=json), json))
        var = 'c->{m}'.format(m=member)
        val = '{}.GetBool()'.format(json)
        sources.add_source(line)
        return VarValue(variable=var, value=val)
    elif t == StandardType.string:
        line = '{i}if({j}.IsString()==false) {err} \n'\
            .format(m=member, i=indent, t=t, n=name, j=json,
                    err=json_return_error("read value for {n} was not a string".format(m=member, i=indent, t=t, n=name, j=json), json))
        var = 'c->{m}'.format(m=member)
        val = '{}.GetString()'.format(json)
        sources.add_source(line)
        return VarValue(variable=var, value=val)
    else:
        print('BUG: No type specified')
        return VarValue(variable=member, value='bug_unhandled_std_type')


def get_cpp_parse_from_rapidjson(sources: Out, t: StandardType, member: str, indent: str, name: str, member_type: Member) -> None:
    if member_type.is_dynamic_array:
        line = '{i}const rapidjson::Value& arr = iter->value;\n' \
               '{i}if(!arr.IsArray()) {err}\n' \
               '{i}for (rapidjson::SizeType i=0; i<arr.Size(); i++)\n' \
               '{i}{{\n' \
            .format(i=indent, name=name,
                    err=json_return_error("tried to read {name} but value was not a array".format(i=indent, name=name), 'arr'))
        sources.add_source(line)
        vv = get_cpp_parse_from_rapidjson_base(sources, t, member, indent + '  ', name, 'arr[i]')
        sources.add_source('{i}  {var}.push_back({val});\n'.format(i=indent, var=vv.variable, val=vv.value))
        sources.add_source('{}}}\n'.format(indent))
    else:
        vv = get_cpp_parse_from_rapidjson_base(sources, t, member, indent, name, 'iter->value')
        if member_type.is_optional:
            sources.add_source('{i}{var} = std::make_shared<{cpptype}>({val});\n'.format(i=indent, var=vv.variable, val=vv.value, cpptype=t.get_cpp_type()))
        else:
            sources.add_source('{i}{var} = {val};\n'.format(i=indent, var=vv.variable, val=vv.value))


def write_json_member(m: Member, sources: Out, indent):
    if m.typename.standard_type != StandardType.INVALID:
        get_cpp_parse_from_rapidjson(sources, m.typename.standard_type, m.name, indent, m.name, m)
    else:
        if m.is_dynamic_array:
            lines = []
            lines.append('{i}const rapidjson::Value& arr = iter->value;\n')
            lines.append('{i}if(!arr.IsArray()) {err}\n')
            lines.append('{i}for (rapidjson::SizeType i=0; i<arr.Size(); i++)\n')
            lines.append('{i}{{\n')
            lines.append('{i}  {type} temp;\n')
            lines.append('{i}  gaf_ss.str("");\n')
            lines.append('{i}  gaf_ss << gaf_path << ".{name}[" << i << "]";\n')
            lines.append('{i}  {rv} r = ReadFromJsonValue(&temp,arr[i], gaf_ss.str());\n')
            lines.append('{i}  if(r{false}) {{ return r; }}\n')
            lines.append('{i}  c->{name}.push_back(temp);\n')
            lines.append('{i}}}\n')
            for line in lines:
                sources.add_source(line.format(i=indent, name=m.name, type=m.typename.name,
                                               err=json_return_error("tried to read {name} but value was not a array".format(i=indent, name=m.name, type=m.typename.name), 'arr'),
                                               false=json_is_false(),
                                               rv=json_return_value()))
        elif m.is_optional:
            lines = []
            lines.append('{i}c->{name} = std::make_shared<{type}>();\n')
            lines.append('{i}gaf_ss.str("");\n')
            lines.append('{i}gaf_ss << gaf_path << ".{name}";\n')
            lines.append('{i}{rv} r = ReadFromJsonValue(c->{name}.get(),iter->value, gaf_ss.str());\n')
            lines.append('{i}if(r{false})\n')
            lines.append('{i}{{\n')
            lines.append('{i}  c->{name}.reset();\n')
            lines.append('{i}  return r;\n')
            lines.append('{i}}}\n')
            for line in lines:
                sources.add_source(line.format(i=indent, name=m.name, type=m.typename.name,false=json_is_false(), rv=json_return_value()))
        else:
            lines = []
            lines.append('{i}gaf_ss.str("");\n')
            lines.append('{i}gaf_ss << gaf_path << ".{name}";\n')
            lines.append('{i}{rv} r = ReadFromJsonValue(&c->{name},iter->value, gaf_ss.str());\n')
            lines.append('{i}if(r{false})\n')
            lines.append('{i}{{\n')
            lines.append('{i}  return r;\n')
            lines.append('{i}}}\n')
            for line in lines:
                sources.add_source(line.format(i=indent, name=m.name, false=json_is_false(), rv=json_return_value()))


def write_json_source_for_cpp(sources: Out, s: Struct):
    sources.add_source('{rv} ReadFromJsonValue({struct}* c, const rapidjson::Value& value, const std::string& gaf_path) {{\n'.format(struct=s.name, rv=json_return_value()))
    sources.add_source('  std::stringstream gaf_ss;\n')
    sources.add_source('  if(!value.IsObject()) {error}\n'.format(error=json_return_error("tried to read {} but value was not a object".format(s.name), 'value')))
    sources.add_source('  rapidjson::Value::ConstMemberIterator iter;\n')
    for m in s.members:
        sources.add_source('  iter = value.FindMember("{n}");\n'.format(n=m.name))
        sources.add_source('  if(iter != value.MemberEnd()) {\n')
        write_json_member(m, sources, '    ')
        sources.add_source('  }\n')
        if m.missing_is_fail or m.is_optional:
            sources.add_source('  else {\n')
            if m.is_optional:
                sources.add_source('    c->{}.reset();\n'.format(m.name))
            else:
                sources.add_source('    {error}\n'.format(error=json_return_error("missing {} in json object".format(m.name), 'value')))
            sources.add_source('  }\n')
    sources.add_source('  return {};\n'.format(json_return_ok()))
    sources.add_source('}\n')
    sources.add_source('\n')


def determine_pushback_value(m: Member) -> str:
    t = m.typename
    if t.standard_type == StandardType.string:
        return '""'
    tl = TypeList()
    tl.add_default_types()
    if tl.is_valid_type(t.name):
        nt = tl.get_type(t.name)
        return nt.default_value
    else:
        return '{}()'.format(t.name)


def write_single_imgui_member_to_source(name: str, var: str, t: StandardType, sources: Out, indent: str, m: Member, add_delete: bool, opt: ImguiOptions):
    if t == StandardType.int8:
        sources.add_source('{i}ImGui::Edit({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.int16:
        sources.add_source('{i}ImGui::Edit({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.int32:
        sources.add_source('{i}ImGui::InputInt({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.int64:
        sources.add_source('{i}ImGui::Edit({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.uint8:
        sources.add_source('{i}ImGui::Edit({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.uint16:
        sources.add_source('{i}ImGui::Edit({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.uint32:
        sources.add_source('{i}ImGui::Edit({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.uint64:
        sources.add_source('{i}ImGui::Edit({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.float:
        sources.add_source('{i}ImGui::InputFloat({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.double:
        sources.add_source('{i}ImGui::InputDouble({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.byte:
        sources.add_source('{i}ImGui::Edit({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.bool:
        sources.add_source('{i}ImGui::Checkbox({name}, {var});\n'.format(name=name, var=var, i=indent))
    elif t == StandardType.string:
        sources.add_source('{i}{{\n'.format(i=indent))
        sources.add_source('{i}  char gaf_temp[1024];\n'.format(i=indent))
        sources.add_source('{i}  strcpy(gaf_temp, ({var})->c_str());\n'.format(var=var, i=indent))
        sources.add_source('{i}  if(ImGui::InputText({name}, gaf_temp, 1024))\n'.format(name=name, i=indent))
        sources.add_source('{i}  {{\n'.format(i=indent))
        sources.add_source('{i}    *({var}) = gaf_temp;\n'.format(var=var, i=indent))
        sources.add_source('{i}  }}\n'.format(i=indent))
        sources.add_source('{i}}}\n'.format(i=indent))
    else:
        if m.typename.is_enum:
            sources.add_source('{i}RunImgui({var}, {name});\n'.format(var=var, i=indent, name=name))
        else:
            sources.add_source('{i}if(ImGui::TreeNodeEx({name}, ImGuiTreeNodeFlags_DefaultOpen{extra_flags}))\n'
                               .format(name=name, i=indent,
                                       extra_flags='' if not add_delete else '| ImGuiTreeNodeFlags_FramePadding'))
            sources.add_source('{i}{{\n'.format(i=indent))
            if add_delete:
                sources.add_source('{i}  ImGui::SameLine();\n'.format(i=indent))
                add_imgui_delete_button(m, sources, opt)
            sources.add_source('{i}  RunImgui({var});\n'.format(var=var, i=indent))
            sources.add_source('{i}  ImGui::TreePop();\n'.format(i=indent))
            sources.add_source('{i}}}\n'.format(i=indent))
            if add_delete:
                sources.add_source('{i}else\n'.format(i=indent))
                sources.add_source('{i}{{\n'.format(i=indent))
                sources.add_source('{i}  ImGui::SameLine();\n'.format(i=indent))
                add_imgui_delete_button(m, sources, opt)
                sources.add_source('{i}}}\n'.format(i=indent))


def determine_new_value(m: Member) -> str:
    t = m.typename
    tl = TypeList()
    tl.add_default_types()
    if tl.is_valid_type(t.name) and t.standard_type != StandardType.string:
        nt = tl.get_type(t.name)
        return 'new {t}({val})'.format(t=t.get_cpp_type(), val=nt.default_value)
    else:
        return 'new {}()'.format(t.name)


def add_imgui_delete_button(m: Member, sources: Out, opt: ImguiOptions):
    sources.add_source('      if( ImGui::Button({delete}) )\n'.format(delete=opt.imgui_remove))
    sources.add_source('      {\n')
    sources.add_source('        delete_index = i;\n')
    sources.add_source('        please_delete = true;\n')
    sources.add_source('      }\n')


def write_single_member_to_source(m: Member, sources: Out, opt: ImguiOptions):
    if not m.is_dynamic_array:
        if m.is_optional:
            sources.add_source('    if(c->{var})\n'.format(var=m.name))
            sources.add_source('    {\n')
            write_single_imgui_member_to_source('"{}"'.format(m.name), 'c->{}.get()'.format(m.name), m.typename.standard_type, sources, '      ', m, False, opt)
            sources.add_source('      if(ImGui::Button("Clear {name}")) {{ c->{name}.reset(); }}\n'.format(name=m.name))
            sources.add_source('    }\n')
            sources.add_source('    else\n')
            sources.add_source('    {\n')
            sources.add_source('      if(ImGui::Button("Set {name}")) {{ c->{name}.reset({new_val}); }}\n'
                               .format(name=m.name, new_val=determine_new_value(m)))
            sources.add_source('    }\n')
            sources.add_source('    \n'.format(var=m.name))
        else:
            write_single_imgui_member_to_source('"{}"'.format(m.name), '&c->{}'.format(m.name), m.typename.standard_type, sources, '  ', m, False, opt)
    else:
        short_version = m.typename.standard_type != StandardType.INVALID or m.typename.is_enum
        sources.add_source('  if(ImGui::TreeNodeEx("{name}", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))\n'.format(name=m.name, var=m.name))
        sources.add_source('  {\n')
        sources.add_source('    ImGui::SameLine();\n'.format(name=m.name))
        sources.add_source('    if(ImGui::Button({add}))\n'.format(add=opt.imgui_add))
        sources.add_source('    {\n')
        sources.add_source('      c->{var}.push_back({val});\n'.format(var=m.name, val=determine_pushback_value(m)))
        sources.add_source('    }\n')
        sources.add_source('    std::size_t delete_index = 0;\n')
        sources.add_source('    bool please_delete = false;\n')
        sources.add_source('    for(std::size_t i=0; i<c->{var}.size(); i+= 1)\n'.format(var=m.name))
        sources.add_source('    {\n')
        sources.add_source('      std::stringstream gaf_ss;\n')
        sources.add_source('      gaf_ss << "{name}[" << i << "]";\n'.format(name=m.name))
        sources.add_source('      ImGui::PushID(i);\n')
        write_single_imgui_member_to_source('gaf_ss.str().c_str()', '&c->{}[i]'.format(m.name), m.typename.standard_type, sources, '      ', m, not short_version, opt)
        if short_version:
            sources.add_source('      ImGui::SameLine();\n')
            add_imgui_delete_button(m, sources, opt)
        sources.add_source('      ImGui::PopID();\n')
        sources.add_source('    }\n')
        sources.add_source('    if(please_delete)\n')
        sources.add_source('    {\n')
        sources.add_source('      c->{var}.erase(c->{var}.begin()+delete_index);\n'.format(var=m.name))
        sources.add_source('    }\n')
        sources.add_source('    ImGui::TreePop();\n')
        sources.add_source('  }\n')
        sources.add_source('  else\n')
        sources.add_source('  {\n')
        sources.add_source('    ImGui::SameLine();\n'.format(name=m.name))
        sources.add_source('    if(ImGui::Button({add}))\n'.format(add=opt.imgui_add))
        sources.add_source('    {\n')
        sources.add_source('      c->{var}.push_back({val});\n'.format(var=m.name, val=determine_pushback_value(m)))
        sources.add_source('    }\n')
        sources.add_source('  }\n')


def write_imgui_source_for_cpp(sources: Out, s: Struct, opt: ImguiOptions):
    sources.add_source('void RunImgui({name}* c)\n'.format(name=s.name))
    sources.add_source('{\n')
    for m in s.members:
        write_single_member_to_source(m, sources, opt)
    sources.add_source('}\n')
    sources.add_source('\n')


def write_member_variables_for_cpp(sources: Out, s: Struct):
    # sources.add_header(' public:\n')
    for m in s.members:

        # m.typename.is_enum
        type_name = m.typename.name
        if m.is_optional:
            sources.add_header('  std::shared_ptr<{tn}> {n};\n'.format(n=m.name, tn=type_name))
        elif m.is_dynamic_array:
            sources.add_header('  std::vector<{tn}> {n};\n'.format(n=m.name, tn=type_name))
        else:
            sources.add_header('  {tn} {n};\n'.format(n=m.name, tn=type_name))
    sources.add_header('}}; // class {}\n'.format(s.name))


def write_default_constructor_for_cpp(s: Struct, sources: Out):
    common_members = [x for x in s.members if x.defaultvalue is not None]
    if len(common_members) > 0:
        sources.add_header('  {}();\n'.format(s.name))
        sources.add_header('\n')
        sources.add_source('{n}::{n}()\n'.format(n=s.name))
        sep = ':'
        for m in common_members:
            default_value = m.defaultvalue
            if m.typename.is_enum:
                default_value = '{t}::{v}'.format(t=m.typename.name, v=m.defaultvalue)
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


def get_value_prefix_opt(e: Enum) -> str:
    return '{}::'.format(e.name)


def add_enum_json_function(e: Enum, sources: Out, type_enum: bool=False):
    enum_type = '{}::Type'.format(e.name) if type_enum else e.name
    value_prefix = get_value_prefix_opt(e)
    arg = ', const std::string& gaf_path'
    sources.add_header('{rv} ReadFromJsonValue({t}* c, const rapidjson::Value& value{a});\n'.format(t=enum_type, rv=json_return_value(), a=arg))
    sources.add_source('{rv} ReadFromJsonValue({t}* c, const rapidjson::Value& value{a})\n'.format(t=enum_type, rv=json_return_value(), a=arg))
    sources.add_source('{\n')
    sources.add_source('  std::stringstream gaf_ss;\n')
    sources.add_source('  if(value.IsString()==false) {err};\n'.format(err=json_return_error("read value for {e} was not a string".format(e=e.name), 'value')))
    for v in e.values:
        sources.add_source('  if(strcmp(value.GetString(), "{v}")==0) {{ *c = {p}{v}; return {ok};}}\n'.format(p=value_prefix, v=v, ok=json_return_ok()))
    sources.add_source('  {}\n'.format(json_return_error("read string for {e} was not valid".format(e=e.name), 'value')))
    sources.add_source('}\n')
    sources.add_source('\n')


def generate_json(f: File, name: str) -> Out:
    sources = Out()
    headerguard = 'GENERATED_JSON_' + name.upper()
    
    sources.add_header('#ifndef {}_H\n'.format(headerguard))
    sources.add_header('#define {}_H\n'.format(headerguard))
    sources.add_header('\n')

    sources.add_header('#include <string>\n')
    sources.add_source('#include <cstring>\n')
    sources.add_header('#include "rapidjson/document.h"\n')
    sources.add_header('\n')
    sources.add_header('#include "gaf_{}.h"\n'.format(name))
    sources.add_header('\n')

    if f.package_name != '':
        sources.add_header('namespace {} {{\n'.format(f.package_name))
        sources.add_header('\n')

    if len(f.typedefs) > 0:
        for s in f.typedefs:
            sources.add_header('class {};\n'.format(s.name))
        sources.add_header('\n')

    for e in f.enums:
        add_enum_json_function(e, sources)

    for s in f.structs_defined:
        write_json_source_for_cpp(sources, s)
        
        sources.add_header('\n')
        arg = ', const std::string& gaf_path'
        sources.add_header('{rv} ReadFromJsonValue({name}* c, const rapidjson::Value& value{a});\n'.format(name=s.name, rv=json_return_value(), a=arg))
        sources.add_header('\n')
    
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

    if f.package_name != '':
        sources.add_header('}} // namespace {}\n'.format(f.package_name))
        sources.add_header('\n')
    sources.add_header('\n')
    sources.add_header('#endif  // {}_H\n'.format(headerguard))

    return sources


def generate_imgui(f: File, name: str, opt: ImguiOptions) -> Out:
    sources = Out()
    headerguard = 'GENERATED_IMGUI_' + name.upper()

    sources.add_header('#ifndef {}_H\n'.format(headerguard))
    sources.add_header('#define {}_H\n'.format(headerguard))
    sources.add_header('\n')
    sources.add_header('#include "gaf_{}.h"\n'.format(name))
    sources.add_header('\n')

    if f.package_name != '':
        sources.add_header('namespace {} {{\n'.format(f.package_name))
        sources.add_header('\n')

    if len(f.typedefs) > 0:
        for s in f.typedefs:
            sources.add_header('class {};\n'.format(s.name))
        sources.add_header('\n')

    for e in f.enums:
        sources.add_header('const char* ToString({name} en);\n'.format(name=e.name))
        sources.add_source('const char* ToString({name} en)\n'.format(name=e.name))
        sources.add_source('{\n')
        for v in e.values:
            sources.add_source('  if(en == {prefix}{val}) {{ return "{name}"; }}\n'.format(prefix=get_value_prefix_opt(e, opt), name=v, val=v))
        sources.add_source('  return "<invalid value>";\n')
        sources.add_source('}\n')

        sources.add_header('void RunImgui({name}* en, const char* label);\n'.format(name=e.name))
        sources.add_source('void RunImgui({name}* en, const char* label)\n'.format(name=e.name))
        sources.add_source('{\n')
        sources.add_source('  if(ImGui::BeginCombo(label, ToString(*en)))\n')
        sources.add_source('  {\n')
        for v in e.values:
            sources.add_source('    if(ImGui::Selectable("{name}", *en == {prefix}{val})) {{ *en = {prefix}{val}; }}\n'.format(prefix=get_value_prefix_opt(e, opt), name=v, val=v))
        sources.add_source('    ImGui::EndCombo();\n')
        sources.add_source('  }\n')
        sources.add_source('}\n')
        sources.add_header('\n')
        sources.add_source('\n')
    
    for s in f.structs_defined:
        write_imgui_source_for_cpp(sources, s, opt)

        sources.add_header('\n')
        sources.add_header('void RunImgui({name}* c);\n'.format(name=s.name))
    
    if f.package_name != '':
        sources.add_header('}} // namespace {}\n'.format(f.package_name))
        sources.add_header('\n')
    sources.add_header('\n')
    sources.add_header('#endif  // {}_H\n'.format(headerguard))

    return sources


def generate_cpp(f: File, name: str) -> Out:
    sources = Out()

    headerguard = 'GENERATED_' + name.upper()

    # get all standard types used for typedefing later on...
    unique_types = get_unique_types(f)
    default_types = [t for t in unique_types
                     if t.standard_type.get_cpp_type() != '' and t.name != t.get_cpp_type()
                     ]

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
    if has_string:
        added_include = True
        sources.add_header('#include <string>\n')
        
    if has_dynamic_arrays:
        added_include = True
        sources.add_header('#include <vector>\n')
    if has_optional:
        added_include = True
        sources.add_header('#include <memory>\n')
    if added_include:
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
        sources.add_header('enum class {} {{\n'.format(e.name))
        iterate_enum(e, sources)
        sources.add_header('}}; // enum {}\n'.format(e.name))

        sources.add_header('\n')

    for s in f.structs_defined:
        sources.add_header('class {} {{\n'.format(s.name))
        sources.add_header(' public:\n')
        write_default_constructor_for_cpp(s, sources)

        write_member_variables_for_cpp(sources, s)
        sources.add_header('\n')

    if f.package_name != '':
        sources.add_header('}} // namespace {}\n'.format(f.package_name))
        sources.add_header('\n')
    sources.add_header('\n')
    sources.add_header('#endif  // {}_H\n'.format(headerguard))

    return sources


def write_cpp(sources: Out, out_dir: str, name: str, prefix: str, package_name: str, includes: typing.List[str]):
    with open(os.path.join(out_dir, prefix + name + '.h'), 'w', encoding='utf-8') as out:
        for s in sources.header:
            out.write(s)

    with open(os.path.join(out_dir, prefix + name + '.cc'), 'w', encoding='utf-8') as out:
        out.write('#include "{}.h"\n'.format(prefix + name))
        out.write('\n')
        for inc in includes:
            out.write('#include {}\n'.format(inc))
        out.write('\n')

        if package_name != '':
            out.write('namespace {} {{\n'.format(package_name))
            out.write('\n')

        for s in sources.source:
            out.write(s)

        if package_name != '':
            out.write('}} // namespace {}\n'.format(package_name))
            out.write('\n')


class CppPlugin(Plugin):
    def get_name(self) -> str:
        return "cpp"

    def add_arguments(self, parser):
        pass
    
    def run_plugin(self, parsed_file: File, name: str, args):
        out = generate_cpp(parsed_file, name)
        write_cpp(out, args.output_folder, name, 'gaf_', parsed_file.package_name, ['<limits>'])


class RapidJsonPlugin(Plugin):
    def get_name(self) -> str:
        return "rapidjson"

    def add_arguments(self, parser):
        pass
    
    def run_plugin(self, parsed_file: File, name: str, args):
        out = generate_json(parsed_file, name)
        write_cpp(out, args.output_folder, name, 'gaf_rapidjson_', parsed_file.package_name, ['<sstream>', '"rapidjson/document.h"'])



class ImguiPlugin(Plugin):
    def get_name(self) -> str:
        return "imgui"

    def add_arguments(self, parser):
        parser.add_argument('--imgui-headers', nargs='+', help='use this header instead of the standard imgui')
        parser.add_argument('--imgui-add', help='the imgui add item button text')
        parser.add_argument('--imgui-remove', help='the imgui remove item button text')
    
    def run_plugin(self, parsed_file: File, name: str, args):
        imgui_headers = args.imgui_headers if args.imgui_headers is not None else ['"imgui.h"']
        imgui_add = args.imgui_add if args.imgui_add is not None else '"Add"'
        imgui_remove = args.imgui_remove if args.imgui_remove is not None else '"Remove"'
        
        out = generate_imgui(parsed_file, name, ImguiOptions(imgui_add, imgui_remove))
        write_cpp(out, args.output_folder, name, 'gaf_imgui_', parsed_file.package_name, imgui_headers)

