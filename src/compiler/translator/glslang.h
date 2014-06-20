//
// Copyright (c) 2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

namespace sh
{
struct TParseContext;
}

extern int glslang_initialize(sh::TParseContext* context);
extern int glslang_finalize(sh::TParseContext* context);

extern int glslang_scan(size_t count,
                        const char* const string[],
                        const int length[],
                        sh::TParseContext* context);
extern int glslang_parse(sh::TParseContext* context);

