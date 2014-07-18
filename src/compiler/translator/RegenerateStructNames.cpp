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

    if (userType->name().find("_webgl") == 0)
        return;
    TString tmp = "_webgl_struct_" + TString(Str(userType->uniqueId()).c_str());
    tmp += "_" + userType->name();
    userType->setName(tmp);
}
