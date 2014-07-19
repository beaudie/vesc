//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/RegenerateStructNames.h"
#include "compiler/translator/compilerdebug.h"

void RegenerateStructNames::visitSymbol(TIntermSymbol *symbol)
{
    ASSERT(symbol);
    TType *type = symbol->getTypePointer();
    ASSERT(type);
    TStructure *userType = type->getStruct();
    if (!userType)
        return;

    // Map {name} to _webgl_struct_{uniqueId}_{name}.
    const char kPrefix[] = "_webgl_struct_";
    if (userType->name().find(kPrefix) == 0)
    {
        // The name has already been regenerated.
        return;
    }
    TString tmp = kPrefix + TString(Str(userType->uniqueId()).c_str());
    tmp += "_" + userType->name();
    userType->setName(tmp);
}
