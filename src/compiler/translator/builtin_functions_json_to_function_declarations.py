#!/usr/bin/python
# input:
# {
#     "name": "pow",
#     "level": "COMMON_BUILTINS",
#     "op": "auto",
#     "returnType": {"basic": "Float", "genType": "yes"},
#     "parameters":
#     [
#         {"basic": "Float", "genType": "yes"},
#         {"basic": "Float", "genType": "yes"}
#     ]
# }

# output:
# COMMON_BUILTINS, auto
#   genType pow(genType, genType);

import json
from collections import OrderedDict
import os

def set_working_dir():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

set_working_dir()

def is_matrix(typeObj):
    return typeObj['secondarySize'] > 1

def is_vector(typeObj):
    return typeObj['secondarySize'] == 1 and typeObj['primarySize'] > 1

def get_type_string(typeObj, allowUndefinedPrecision=True):
    if 'vecSize' in typeObj:
        typeObj['primarySize'] = typeObj['vecSize']
        typeObj['secondarySize'] = 1
    elif 'columns' in typeObj:
        typeObj['primarySize'] = typeObj['columns']
        typeObj['secondarySize'] = typeObj['rows']
    elif 'primarySize' not in typeObj:
        typeObj['primarySize'] = 1
        typeObj['secondarySize'] = 1
    if 'precision' not in typeObj:
        if typeObj['basic'] != 'Bool' and not allowUndefinedPrecision:
            raise Exception('Undefined precision for a non-bool variable')
        typeObj['precision'] = 'Undefined'
    if 'qualifier' not in typeObj:
        typeObj['qualifier'] = 'Global'
    template_type = 'StaticType::Get<Ebt{basic}, Ebp{precision}, Evq{qualifier}, {primarySize}, {secondarySize}>()'
    return template_type.format(**typeObj)

def get_suffix(props):
    if 'suffix' in props:
        return props['suffix']
    return ''

def get_extension(props):
    if 'extension' in props:
        return props['extension']
    return 'UNDEFINED'

def get_op(name, props):
    if 'op' in props:
        if props['op'] == 'auto':
            return name[0].upper() + name[1:]
        return props['op']
    return 'CallBuiltInFunction'

def get_known_to_not_have_side_effects(props):
    if 'op' in props:
        if 'hasSideEffects' in props:
            return 'false'
        else:
            for param in get_parameters(props):
                if 'qualifier' in param and param['qualifier'] == 'Out':
                    return 'false'
            return 'true'
    return 'false'

def get_parameters(function_props):
    if 'parameters' in function_props:
        return function_props['parameters']
    return []

genTypes = {
    'Float': 'genType',
    'Int': 'genIType',
    'UInt': 'genUType',
    'Bool': 'genBType'
}
genVecTypes = {
    'Float': 'vec',
    'Int': 'ivec',
    'UInt': 'uvec',
    'Bool': 'bvec'
}

def output_type(type):
    get_type_string(type)
    basic = type['basic']
    qual = ''
    if type['qualifier'] == 'Out':
        qual = 'out '
    if 'genType' in type:
        if type['genType'] == 'yes':
            return qual + genTypes[basic]
        elif type['genType'] == 'vec':
            return qual + genVecTypes[basic]
        elif type['genType'] == 'sampler':
            raise Exception('Unimplemented')
    if is_matrix(type):
        if type['primarySize'] == type['secondarySize']:
            return qual + 'mat' + str(type['primarySize'])
        return qual + 'mat' + str(type['primarySize']) + 'x' + str(type['secondarySize'])
    if is_vector(type):
        if (basic == 'Float'):
            return qual + 'vec' + str(type['primarySize'])
        return qual + basic[0].lower() + 'vec' + str(type['primarySize'])
    return qual + basic.lower()

output_lines = []

def process_function_group(group_name, group):
    if 'condition' in group:
        pass
    output_lines.append('GROUP BEGIN ' + group_name)
    prev_level = ''
    prev_op = ''
    if 'functions' in group:
        for props in group['functions']:
            function_name = props['name']
            name_suffix = get_suffix(props)
            extension = get_extension(props)
            op = get_op(function_name, props)
            if 'op' in props and props['op'] == 'auto':
                op = 'auto'

            params = get_parameters(props)
            params = [output_type(param) for param in params]
            params = ', '.join(params)

            template_fun = '  {level}, {op}\n    {return_type} {name}({params}); {metadata}'

            if props['level'] == prev_level and op == prev_op:
                template_fun = '    {return_type} {name}({params}); {metadata}'

            metadata = ''
            if 'suffix' in props:
                metadata = {'suffix' : str(get_suffix(props))}
            if metadata != '':
                metadata = json.dumps(metadata)

            output_lines.append(template_fun.format(level = props['level'], op = op, return_type = output_type(props['returnType']), name = function_name, params = params, metadata = metadata).rstrip())
            prev_level = props['level']
            prev_op = op
    if 'subgroups' in group:
        for subgroup_name, subgroup in group['subgroups'].iteritems():
            process_function_group(subgroup_name, subgroup)
    output_lines.append('GROUP END ' + group_name)
    output_lines.append('')


parsed_functions = {}
with open('builtin_functions.json') as f:
    parsed_functions = json.load(f, object_pairs_hook=OrderedDict)

for group_name, group in parsed_functions.iteritems():
    process_function_group(group_name, group)


with open('builtin_function_declarations.txt', 'w') as f:
    f.write('\n'.join(output_lines))

