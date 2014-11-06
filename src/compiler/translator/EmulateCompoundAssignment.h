//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_EMULATE_COMPOUND_ASSIGNMENT_H_
#define COMPILER_EMULATE_COMPOUND_ASSIGNMENT_H_

#include "common/angleutils.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"

// This class gathers all compound assignments from the AST and can then write
// the functions required for their precision emulation. This way there is no
// need to write a huge number of variations of the emulated compound assignment
// to every translated shader with emulation enabled.

class EmulateCompoundAssignment : public TIntermTraverser
{
  public:
    EmulateCompoundAssignment() { }

    virtual bool visitBinary(Visit visit, TIntermBinary *);

    void writeEmulation(TInfoSinkBase& sink);

  private:
    DISALLOW_COPY_AND_ASSIGN(EmulateCompoundAssignment);

    struct TypePair
    {
        TypePair(const char *l, const char *r)
            : lType(l), rType(r) { }

        const char *lType;
        const char *rType;
    };

    struct TypePairCompare
    {
        bool operator() (const TypePair& l, const TypePair& r) const
        {
            if (l.lType == r.lType)
                return l.rType < r.rType;
            return l.lType < r.lType;
        }
    };

    typedef std::set<TypePair, TypePairCompare> EmulationSet;

    EmulationSet mEmulateAdd;
    EmulationSet mEmulateSub;
    EmulationSet mEmulateMul;
    EmulationSet mEmulateDiv;
};

#endif  // COMPILER_EMULATE_COMPOUND_ASSIGNMENT_H_
