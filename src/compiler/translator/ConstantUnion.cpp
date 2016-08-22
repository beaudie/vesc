//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConstantUnion: Constant folding helper class.

#include "compiler/translator/ConstantUnion.h"

#include "base/numerics/safe_math.h"

bool TConstantUnion::cast(TBasicType newType, const TConstantUnion &constant)
{
    switch (newType)
    {
        case EbtFloat:
            switch (constant.type)
            {
                case EbtInt:
                    setFConst(static_cast<float>(constant.getIConst()));
                    break;
                case EbtUInt:
                    setFConst(static_cast<float>(constant.getUConst()));
                    break;
                case EbtBool:
                    setFConst(static_cast<float>(constant.getBConst()));
                    break;
                case EbtFloat:
                    setFConst(static_cast<float>(constant.getFConst()));
                    break;
                default:
                    return false;
            }
            break;
        case EbtInt:
            switch (constant.type)
            {
                case EbtInt:
                    setIConst(static_cast<int>(constant.getIConst()));
                    break;
                case EbtUInt:
                    setIConst(static_cast<int>(constant.getUConst()));
                    break;
                case EbtBool:
                    setIConst(static_cast<int>(constant.getBConst()));
                    break;
                case EbtFloat:
                    setIConst(static_cast<int>(constant.getFConst()));
                    break;
                default:
                    return false;
            }
            break;
        case EbtUInt:
            switch (constant.type)
            {
                case EbtInt:
                    setUConst(static_cast<unsigned int>(constant.getIConst()));
                    break;
                case EbtUInt:
                    setUConst(static_cast<unsigned int>(constant.getUConst()));
                    break;
                case EbtBool:
                    setUConst(static_cast<unsigned int>(constant.getBConst()));
                    break;
                case EbtFloat:
                    setUConst(static_cast<unsigned int>(constant.getFConst()));
                    break;
                default:
                    return false;
            }
            break;
        case EbtBool:
            switch (constant.type)
            {
                case EbtInt:
                    setBConst(constant.getIConst() != 0);
                    break;
                case EbtUInt:
                    setBConst(constant.getUConst() != 0);
                    break;
                case EbtBool:
                    setBConst(constant.getBConst());
                    break;
                case EbtFloat:
                    setBConst(constant.getFConst() != 0.0f);
                    break;
                default:
                    return false;
            }
            break;
        case EbtStruct:  // Struct fields don't get cast
            switch (constant.type)
            {
                case EbtInt:
                    setIConst(constant.getIConst());
                    break;
                case EbtUInt:
                    setUConst(constant.getUConst());
                    break;
                case EbtBool:
                    setBConst(constant.getBConst());
                    break;
                case EbtFloat:
                    setFConst(constant.getFConst());
                    break;
                default:
                    return false;
            }
            break;
        default:
            return false;
    }

    return true;
}

TConstantUnion TConstantUnion::operator*(const TConstantUnion &constant) const
{
    TConstantUnion returnValue;
    assert(type == constant.type);
    switch (type)
    {
        case EbtInt:
        {
            base::CheckedNumeric<int> result(iConst);
            result *= constant.iConst;
            ASSERT(result.IsValid());
            returnValue.setIConst(result.ValueOrDefault(0));
            break;
        }
        case EbtUInt:
            returnValue.setUConst(uConst * constant.uConst);
            break;
        case EbtFloat:
            returnValue.setFConst(fConst * constant.fConst);
            break;
        default:
            assert(false && "Default missing");
    }

    return returnValue;
}
