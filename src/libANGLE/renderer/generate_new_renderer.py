#!/usr/bin/python

import os
import sys
import re
import string

if len(sys.argv) < 3:
    print('Usage: ' + sys.argv[0] + ' <renderer name> <renderer suffix>')

renderer_name = sys.argv[1]
renderer_suffix = sys.argv[2]

# ensure subdir exists
if not os.path.isdir(renderer_name):
    os.mkdir(renderer_name)

impl_classes = [
    'Buffer',
    'Compiler',
    'DefaultAttachment',
    'FenceNV',
    'FenceSync',
    'Framebuffer',
    'Program',
    'Query',
    'Renderbuffer',
    'Shader',
    'Surface',
    'Texture',
    'TransformFeedback',
    'VertexArray',
]

h_file_template = """//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// $TypedImpl.h: Defines the class interface for $TypedImpl.

#ifndef LIBANGLE_RENDERER_${RendererNameCaps}_${TypedImplCaps}_H
#define LIBANGLE_RENDERER_${RendererNameCaps}_${TypedImplCaps}_H

#include "libANGLE/renderer/$BaseImpl.h"

namespace rx
{

class $TypedImpl : public $BaseImpl
{
  public:
    $TypedImpl();
    ~$TypedImpl() override;
$ImplMethodDeclarations
  private:
    DISALLOW_COPY_AND_ASSIGN($TypedImpl);
};

}

#endif // LIBANGLE_RENDERER_${RendererNameCaps}_${TypedImplCaps}_H
"""

cpp_file_template = """//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// $TypedImpl.cpp: Implements the class methods for $TypedImpl.

#include "libANGLE/renderer/$RendererName/$TypedImpl.h"

#include "common/debug.h"

namespace rx
{

$TypedImpl::$TypedImpl()
    : $BaseImpl()
{}

$TypedImpl::~$TypedImpl()
{}
$ImplMethodDefinitions
}
"""

def generate_impl_declaration(impl_stub):
    # ensure the wrapped lines are aligned vertically
    temp = re.sub(r'\n        ', '\n', impl_stub)
    return temp + ' override;\n'

def generate_impl_definition(impl_stub, typed_impl):
    function_signature = impl_stub.strip()

    # strip comments
    function_signature = re.sub(r'\/\/[^\n]*\n', '', function_signature).strip()

    prog = re.compile(r'^([^ ]+) ')
    result = prog.match(function_signature)

    # ensure the wrapped lines are aligned vertically
    spaces = ' ' * len(typed_impl)
    function_signature = re.sub(r'\n          ', '\n' + spaces, function_signature)

    # add class scoping
    function_signature = prog.sub(r'\1 ' + typed_impl + '::', function_signature)
    function_signature += '\n'

    return_statement = ''
    if result.group(1) != 'void':
        if result.group(1) == 'gl::Error':
            return_statement = '    return gl::Error(GL_INVALID_OPERATION);\n'
        elif result.group(1) == 'egl::Error':
            return_statement = '    return egl::Error(EGL_BAD_ACCESS);\n'
        elif result.group(1) == 'LinkResult':
            return_statement = '    return LinkResult(false, gl::Error(GL_INVALID_OPERATION));\n'
        else:
            return_statement = '    return ' + result.group(1) + '();\n'

    body = '{\n' + '    UNIMPLEMENTED();\n' + return_statement +'}\n'

    return '\n' + function_signature + body

for impl_class in impl_classes:

    base_impl = impl_class + 'Impl'
    typed_impl = impl_class + renderer_suffix

    impl_h_file_path = base_impl + '.h'
    h_file_path = os.path.join(renderer_name, typed_impl + '.h')
    cpp_file_path = os.path.join(renderer_name, typed_impl + '.cpp')

    impl_h_file = open(impl_h_file_path, 'r')
    h_file = open(h_file_path, 'w')
    cpp_file = open(cpp_file_path, 'w')

    # extract impl stubs
    copy = False
    impl_stubs = ''
    for line in impl_h_file:
        clean_line = line.strip()
        if '~' + base_impl in clean_line:
            copy = True
        elif ';' in clean_line and ' = 0' not in clean_line:
            copy = False
        elif '}' in clean_line:
            copy = False
        elif 'protected:' in clean_line:
            copy = False
        elif 'private:' in clean_line:
            copy = False
        elif copy:
            impl_stubs += line

    impl_method_declarations = ''
    impl_method_definitions = ''

    for impl_stub in impl_stubs.split(' = 0;\n'):
        if 'virtual' in impl_stub:
            temp = re.sub(r'virtual ', '', impl_stub)
            impl_method_declarations += generate_impl_declaration(temp)
            impl_method_definitions += generate_impl_definition(temp, typed_impl)

    substitutions = {
        'BaseImpl': base_impl,
        'TypedImpl': typed_impl,
        'TypedImplCaps': typed_impl.upper(),
        'RendererName': renderer_name,
        'RendererNameCaps': renderer_name.upper(),
        'ImplMethodDeclarations': impl_method_declarations,
        'ImplMethodDefinitions': impl_method_definitions
    }

    h_file.write(string.Template(h_file_template).substitute(substitutions))
    cpp_file.write(string.Template(cpp_file_template).substitute(substitutions))

    impl_h_file.close()
    h_file.close()
    cpp_file.close()
