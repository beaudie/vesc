import argparse
import gzip
import json
import logging
import os
import re
import subprocess
import sys


def ast_cache_name(fn):
    return os.path.basename(fn).replace('.cpp', '.ast.gz')


def build_ast_cache(ast_cache_path, cpp_file):
    ast = json.loads(
        subprocess.check_output([
            'third_party/llvm-build/Release+Asserts/bin/clang++', '-O0', '-Iinclude',
            '-Iutil/capture', '-c', '-Xclang', '-ast-dump=json', cpp_file
        ]).decode())
    ast_filtered = []
    cur_loc = ''
    for entry in ast['inner']:
        # 'loc' seems to be added once at the beginning of each file
        maybe_loc = entry.get('loc', {}).get('file', '')
        if maybe_loc:
            cur_loc = maybe_loc

        # Ignore everything outside of the .cpp
        if not cur_loc.endswith('.cpp'):
            continue

        ast_filtered.append(entry)

    with gzip.open(os.path.join(ast_cache_path, ast_cache_name(cpp_file)), 'wb') as f:
        f.write(json.dumps(ast_filtered).encode())


def check_kind(expr, kind):
    assert expr['kind'] == kind, (kind, expr['kind'])


def checked_inner(expr, kind):
    inner, = expr['inner']
    check_kind(inner, kind)
    return inner


def handle_var(ast, global_vars):
    expr = checked_inner(ast, 'InitListExpr')

    assert ast['name'] not in global_vars
    if ast['name'].startswith('glShaderSource_'):
        literal = checked_inner(checked_inner(expr, 'ImplicitCastExpr'), 'StringLiteral')
        global_vars[ast['name']] = json.loads(literal['value'])
    else:
        values = []
        for inner in expr.get('inner', []):  # Initializer list can be empty
            check_kind(inner, 'ImplicitCastExpr')
            values.append(int(checked_inner(inner, 'IntegerLiteral')['value']))
        global_vars[ast['name']] = values


def skip_implicit_cast(ast):
    if ast['kind'] == 'ImplicitCastExpr':
        ast, = ast['inner']
    return ast


def parse_arg(ast):
    ast = skip_implicit_cast(ast)

    if ast['kind'] == 'IntegerLiteral':
        return int(ast['value'])

    if ast['kind'] == 'DeclRefExpr':
        return {'var_ref': ast['referencedDecl']['name']}

    if ast['kind'] == 'ArraySubscriptExpr':
        array, subscript = ast['inner']
        array = skip_implicit_cast(array)
        # There are a few cases here, one simple case is array[index]:
        if array['kind'] == 'DeclRefExpr' and subscript['kind'] == 'IntegerLiteral':
            return {array['referencedDecl']['name']: int(subscript['value'])}

    return 'fixme:' + ast['kind']


def parse_call(ast):
    inner = skip_implicit_cast(ast['inner'][0])

    check_kind(inner, 'DeclRefExpr')

    name = inner['referencedDecl']['name']
    if name.startswith('t_gl') or name.startswith('t_egl'):
        name = name[2:]  # e.g. t_glBindBuffer -> glBindBuffer

    return [name] + [parse_arg(arg) for arg in ast['inner'][1:]]


def handle_statement(ast, global_vars, calls):
    if ast['kind'] == 'ExprWithCleanups':
        ast, = ast['inner']

    if ast['kind'] == 'CallExpr':
        calls.append(parse_call(ast))
    elif ast['kind'] == 'BinaryOperator':
        calls.append(['TODO: handle assignment'])
    else:
        raise ValueError(ast['kind'])


def parse_file(cpp_file, ast_cache_path):
    with gzip.open(os.path.join(ast_cache_path, ast_cache_name(cpp_file)), 'rb') as f:
        ast = json.loads(f.read().decode())

    functions = {}
    global_vars = {}
    for entry in ast:
        logging.debug('kind=%s name=%s', entry['kind'], entry.get('name', None))

        if entry['kind'] == 'VarDecl':
            handle_var(entry, global_vars)
        elif entry['kind'] == 'FunctionDecl':
            function_name = entry['name']
            calls = []
            for statement in checked_inner(entry, 'CompoundStmt').get('inner', []):
                handle_statement(statement, global_vars, calls)
            assert function_name not in functions
            functions[function_name] = calls
        elif entry['kind'] == 'LinkageSpecDecl':
            assert entry['language'] == 'C'
            for sub_entry in entry.get('inner', []):
                logging.debug('extern_C kind=%s name=%s', sub_entry['kind'],
                              sub_entry.get('name', None))
                # TODO: handle_function()
        else:
            raise ValueError(entry['kind'])

    return global_vars, functions


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--build-ast-cache',
        help='Build AST cache (use existing by default)',
        action='store_true',
        default=False)
    parser.add_argument('--ast-cache-path', help='AST cache path', required=True)
    parser.add_argument('--single-file', help='Process a single file', required=True)
    parser.add_argument('--log', help='Logging level', default='info')
    parser.add_argument('--output', help='JSON output file')

    args = parser.parse_args()
    logging.basicConfig(level=args.log.upper())

    assert os.path.exists(args.ast_cache_path)
    assert args.build_ast_cache or args.output

    if args.build_ast_cache:
        build_ast_cache(args.ast_cache_path, args.single_file)

    if args.output:
        assert args.single_file
        global_vars, functions = parse_file(args.single_file, args.ast_cache_path)

        with open(args.output, 'w') as fo:
            s = json.dumps({'vars': global_vars, 'functions': functions}, indent=2)
            s = re.sub(r'\n\s{8,}', ' ', s)
            s = re.sub(r'\n\s{6,}\]', ' ]', s)
            fo.write(s)


if __name__ == '__main__':
    sys.exit(main())
