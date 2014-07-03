//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_UTIL_H
#define COMPILER_UTIL_H

#include "compiler/translator/Types.h"
#include "angle_gl.h"
#include "common/shadervars.h"

// atof_clamp is like atof but
//   1. it forces C locale, i.e. forcing '.' as decimal point.
//   2. it clamps the value to -FLT_MAX or FLT_MAX if overflow happens.
// Return false if overflow happens.
extern bool atof_clamp(const char *str, float *value);

// If overflow happens, clamp the value to INT_MIN or INT_MAX.
// Return false if overflow happens.
extern bool atoi_clamp(const char *str, int *value);

namespace sh
{

GLenum GLVariableType(const TType &type);
GLenum GLVariablePrecision(const TType &type);
bool IsVaryingIn(TQualifier qualifier);
bool IsVaryingOut(TQualifier qualifier);
bool IsVarying(TQualifier qualifier);
InterpolationType GetInterpolationType(TQualifier qualifier);
TString ArrayString(const TType &type);

template <typename VarT>
inline VarT GetSingleVariableInfo(const TType &type, const TString &name)
{
    ASSERT(!type.getStruct());

    VarT variable;

    variable.type = GLVariableType(type);
    variable.precision = GLVariablePrecision(type);
    variable.name = name.c_str();
    variable.arraySize = static_cast<unsigned int>(type.getArraySize());

    return variable;
}

template <typename VarT, typename CallbackT>
inline VarT GetVariableInfo(const TType &type, const TString &name,
                            std::vector<VarT> *output, CallbackT *callback)
{
    ASSERT(output);
    const TStructure *structure = type.getStruct();

    if (!structure)
    {
        VarT variable = GetSingleVariableInfo<VarT>(type, name);

        if (callback)
        {
            (*callback)(variable);
        }

        output->push_back(variable);

        return variable;
    }
    else
    {
        VarT variable;

        variable.type = GL_STRUCT_ANGLEX;
        variable.name = name.c_str();
        variable.arraySize = static_cast<unsigned int>(type.getArraySize());

        const TFieldList &fields = structure->fields();

        for (size_t fieldIndex = 0; fieldIndex < fields.size(); fieldIndex++)
        {
            TField *field = fields[fieldIndex];
            TType *fieldType = field->type();

            GetVariableInfo(*fieldType, field->name(), &variable.fields, callback);
        }

        if (callback)
        {
            (*callback)(variable);
        }

        output->push_back(variable);

        return variable;
    }
}

}

#endif // COMPILER_UTIL_H
