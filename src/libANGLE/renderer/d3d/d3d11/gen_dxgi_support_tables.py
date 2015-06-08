#!/usr/bin/python
# Copyright 2015 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# gen_dxgi_support_tables.py:
#  Code generation for the DXGI support tables. Determines which formats
#  are natively support in D3D10+.
#
# MSDN links:
#  10_0: https://msdn.microsoft.com/en-us/library/windows/desktop/cc627090.aspx
#  10_1: https://msdn.microsoft.com/en-us/library/windows/desktop/cc627091.aspx
#  11_0: https://msdn.microsoft.com/en-us/library/windows/desktop/ff471325.aspx
#  11_1: https://msdn.microsoft.com/en-us/library/windows/desktop/hh404483.aspx

import sys
import json

macro_prefix = 'F_'

template = """//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// dxgi_support_table:
//   Queries for DXGI support of various texture formats. Depends on DXGI
//   version, D3D feature level, and is sometimes guaranteed or optional.
//

#include "libANGLE/renderer/d3d/d3d11/dxgi_support_table.h"

#include <map>

namespace rx
{{

namespace d3d11
{{

namespace
{{

std::map<DXGI_FORMAT, DXGISupport> g_DXGISupportTable10_0;
std::map<DXGI_FORMAT, DXGISupport> g_DXGISupportTable10_1;
std::map<DXGI_FORMAT, DXGISupport> g_DXGISupportTable11_0;

}} // namespace

#define {prefix}2D D3D11_FORMAT_SUPPORT_TEXTURE2D
#define {prefix}3D D3D11_FORMAT_SUPPORT_TEXTURE3D
#define {prefix}CUBE D3D11_FORMAT_SUPPORT_TEXTURECUBE
#define {prefix}SAMPLE D3D11_FORMAT_SUPPORT_SHADER_SAMPLE
#define {prefix}RT D3D11_FORMAT_SUPPORT_RENDER_TARGET
#define {prefix}MS D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET
#define {prefix}DS D3D11_FORMAT_SUPPORT_DEPTH_STENCIL

void InitializeDXGISupportTables()
{{
{table_data}
}}

#undef {prefix}2D
#undef {prefix}3D
#undef {prefix}CUBE
#undef {prefix}SAMPLE
#undef {prefix}RT
#undef {prefix}MS
#undef {prefix}DS

void DeinitializeDXGISupportTables()
{{
    g_DXGISupportTable10_0.clear();
    g_DXGISupportTable10_1.clear();
    g_DXGISupportTable11_0.clear();
}}

const DXGISupport &GetDXGISupport(DXGI_FORMAT dxgiFormat, D3D_FEATURE_LEVEL featureLevel)
{{
    static UINT AllSupportFlags = D3D11_FORMAT_SUPPORT_TEXTURE2D |
                                  D3D11_FORMAT_SUPPORT_TEXTURE3D |
                                  D3D11_FORMAT_SUPPORT_TEXTURECUBE |
                                  D3D11_FORMAT_SUPPORT_SHADER_SAMPLE |
                                  D3D11_FORMAT_SUPPORT_RENDER_TARGET |
                                  D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET |
                                  D3D11_FORMAT_SUPPORT_DEPTH_STENCIL;
    static const DXGISupport defaultSupport(0, 0, AllSupportFlags);

    switch (featureLevel)
    {{
        case D3D_FEATURE_LEVEL_11_0:
            return g_DXGISupportTable11_0[dxgiFormat];
        case D3D_FEATURE_LEVEL_10_1:
            return g_DXGISupportTable10_1[dxgiFormat];
        case D3D_FEATURE_LEVEL_10_0:
            return g_DXGISupportTable10_0[dxgiFormat];
        default:
            return defaultSupport;
    }}
}}

}} // namespace d3d11

}} // namespace rx
"""

table_init = ""

def do_format(format_data):
    table_data = ""

    json_flag_to_d3d = {
        'texture2D': macro_prefix + '2D',
        'texture3D': macro_prefix + '3D',
        'textureCube': macro_prefix + 'CUBE',
        'shaderSample': macro_prefix + 'SAMPLE',
        'renderTarget': macro_prefix + 'RT',
        'multisampleRT': macro_prefix + 'MS',
        'depthStencil': macro_prefix + 'DS'
    }

    for format_name, format_support in format_data.iteritems():

        always_supported = set()
        never_supported = set()
        optionally_supported = set()
        fl_11_0_supported = set()

        for json_flag, support in format_support.iteritems():

            d3d_flag = [json_flag_to_d3d[json_flag]]

            if support == 'check':
                optionally_supported.update(d3d_flag)
            elif support == 'always':
                always_supported.update(d3d_flag)
            elif support == 'never':
                never_supported.update(d3d_flag)
            elif support == '11_0':
                fl_11_0_supported.update(d3d_flag)
            elif support == '11_1':
                # TODO(jmadill): D3D 11.1 handling
                never_supported.update(d3d_flag)
            elif support == 'dxgi1_2':
                # TODO(jmadill): DXGI 1.2 handling.
                always_supported.update(d3d_flag)
            else:
                print("Data specification error: " + support)
                sys.exit(1)

        for feature_level in ['10_0', '10_1', '11_0']:
            table_name = 'g_DXGISupportTable' + feature_level

            always_for_fl = always_supported
            if feature_level == '11_0':
                always_for_fl = fl_11_0_supported.union(always_for_fl)

            print(always_for_fl)

            always = ' | '.join(always_for_fl)
            never = ' | '.join(never_supported)
            optional = ' | '.join(optionally_supported)

            if not always: always = '0'
            if not never: never = '0'
            if not optional: optional = '0'

            table_data += '    ' + table_name + '[' + format_name + '] =\n'
            table_data += '        DXGISupport(' + always + ', ' + never + ', ' + optional + ');\n'

    return table_data

with open('dxgi_support_data.json') as dxgi_file:
    file_data = dxgi_file.read()
    dxgi_file.close()
    json_data = json.loads(file_data)

    table_data = ""

    for format_data in json_data:
        table_data += do_format(format_data)

    with open('dxgi_support_table.cpp', 'wt') as out_file:
        out_file.write(template.format(prefix=macro_prefix, table_data=table_data))
        out_file.close

