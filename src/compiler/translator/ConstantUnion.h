//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_CONSTANTUNION_H_
#define COMPILER_TRANSLATOR_CONSTANTUNION_H_

#include <assert.h>

#include "compiler/translator/Common.h"
#include "compiler/translator/BaseTypes.h"

class TConstantUnion {
public:
    POOL_ALLOCATOR_NEW_DELETE();
    TConstantUnion()
    {
        iConst = 0;
        type = EbtVoid;
    }

    bool cast(TBasicType newType, const TConstantUnion &constant);

    void setIConst(int i) {iConst = i; type = EbtInt; }
    void setUConst(unsigned int u) { uConst = u; type = EbtUInt; }
    void setFConst(float f) {fConst = f; type = EbtFloat; }
    void setBConst(bool b) {bConst = b; type = EbtBool; }

    int getIConst() const { return iConst; }
    unsigned int getUConst() const { return uConst; }
    float getFConst() const { return fConst; }
    bool getBConst() const { return bConst; }

    bool operator==(const int i) const
    {
        return i == iConst;
    }

    bool operator==(const unsigned int u) const
    {
        return u == uConst;
    }

    bool operator==(const float f) const
    {
        return f == fConst;
    }

    bool operator==(const bool b) const
    {
        return b == bConst;
    }

    bool operator==(const TConstantUnion& constant) const
    {
        if (constant.type != type)
            return false;

        switch (type) {
        case EbtInt:
            return constant.iConst == iConst;
        case EbtUInt:
            return constant.uConst == uConst;
        case EbtFloat:
            return constant.fConst == fConst;
        case EbtBool:
            return constant.bConst == bConst;
        default:
            return false;
        }
    }

    bool operator!=(const int i) const
    {
        return !operator==(i);
    }

    bool operator!=(const unsigned int u) const
    {
        return !operator==(u);
    }

    bool operator!=(const float f) const
    {
        return !operator==(f);
    }

    bool operator!=(const bool b) const
    {
        return !operator==(b);
    }

    bool operator!=(const TConstantUnion& constant) const
    {
        return !operator==(constant);
    }

    bool operator>(const TConstantUnion& constant) const
    { 
        assert(type == constant.type);
        switch (type) {
        case EbtInt:
            return iConst > constant.iConst;
        case EbtUInt:
            return uConst > constant.uConst;
        case EbtFloat:
            return fConst > constant.fConst;
        default:
            return false;   // Invalid operation, handled at semantic analysis
        }
    }

    bool operator<(const TConstantUnion& constant) const
    { 
        assert(type == constant.type);
        switch (type) {
        case EbtInt:
            return iConst < constant.iConst;
        case EbtUInt:
            return uConst < constant.uConst;
        case EbtFloat:
            return fConst < constant.fConst;
        default:
            return false;   // Invalid operation, handled at semantic analysis
        }
    }

    TConstantUnion operator+(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        assert(type == constant.type);
        switch (type) {
        case EbtInt: returnValue.setIConst(iConst + constant.iConst); break;
        case EbtUInt: returnValue.setUConst(uConst + constant.uConst); break;
        case EbtFloat: returnValue.setFConst(fConst + constant.fConst); break;
        default: assert(false && "Default missing");
        }

        return returnValue;
    }

    TConstantUnion operator-(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        assert(type == constant.type);
        switch (type) {
        case EbtInt: returnValue.setIConst(iConst - constant.iConst); break;
        case EbtUInt: returnValue.setUConst(uConst - constant.uConst); break;
        case EbtFloat: returnValue.setFConst(fConst - constant.fConst); break;
        default: assert(false && "Default missing");
        }

        return returnValue;
    }

    TConstantUnion operator*(const TConstantUnion &constant) const;

    TConstantUnion operator%(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        assert(type == constant.type);
        switch (type) {
        case EbtInt: returnValue.setIConst(iConst % constant.iConst); break;
        case EbtUInt: returnValue.setUConst(uConst % constant.uConst); break;
        default:     assert(false && "Default missing");
        }

        return returnValue;
    }

    TConstantUnion operator>>(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        assert(type == constant.type);
        switch (type) {
        case EbtInt: returnValue.setIConst(iConst >> constant.iConst); break;
        case EbtUInt: returnValue.setUConst(uConst >> constant.uConst); break;
        default:     assert(false && "Default missing");
        }

        return returnValue;
    }

    TConstantUnion operator<<(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        // The signedness of the second parameter might be different, but we
        // don't care, since the result is undefined if the second parameter is
        // negative, and aliasing should not be a problem with unions.
        assert(constant.type == EbtInt || constant.type == EbtUInt);
        switch (type) {
        case EbtInt: returnValue.setIConst(iConst << constant.iConst); break;
        case EbtUInt: returnValue.setUConst(uConst << constant.uConst); break;
        default:     assert(false && "Default missing");
        }

        return returnValue;
    }

    TConstantUnion operator&(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        assert(constant.type == EbtInt || constant.type == EbtUInt);
        switch (type) {
        case EbtInt:  returnValue.setIConst(iConst & constant.iConst); break;
        case EbtUInt:  returnValue.setUConst(uConst & constant.uConst); break;
        default:     assert(false && "Default missing");
        }

        return returnValue;
    }

    TConstantUnion operator|(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        assert(type == constant.type);
        switch (type) {
        case EbtInt:  returnValue.setIConst(iConst | constant.iConst); break;
        case EbtUInt:  returnValue.setUConst(uConst | constant.uConst); break;
        default:     assert(false && "Default missing");
        }

        return returnValue;
    }

    TConstantUnion operator^(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        assert(type == constant.type);
        switch (type) {
        case EbtInt:  returnValue.setIConst(iConst ^ constant.iConst); break;
        case EbtUInt:  returnValue.setUConst(uConst ^ constant.uConst); break;
        default:     assert(false && "Default missing");
        }

        return returnValue;
    }

    TConstantUnion operator&&(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        assert(type == constant.type);
        switch (type) {
        case EbtBool: returnValue.setBConst(bConst && constant.bConst); break;
        default:     assert(false && "Default missing");
        }

        return returnValue;
    }

    TConstantUnion operator||(const TConstantUnion& constant) const
    { 
        TConstantUnion returnValue;
        assert(type == constant.type);
        switch (type) {
        case EbtBool: returnValue.setBConst(bConst || constant.bConst); break;
        default:     assert(false && "Default missing");
        }

        return returnValue;
    }

    TBasicType getType() const { return type; }
private:

    union  {
        int iConst;  // used for ivec, scalar ints
        unsigned int uConst; // used for uvec, scalar uints
        bool bConst; // used for bvec, scalar bools
        float fConst;   // used for vec, mat, scalar floats
    } ;

    TBasicType type;
};

#endif // COMPILER_TRANSLATOR_CONSTANTUNION_H_
