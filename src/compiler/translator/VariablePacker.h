//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VARIABLEPACKER_H_
#define COMPILER_TRANSLATOR_VARIABLEPACKER_H_

#include <vector>

#include <GLSLANG/ShaderLang.h>

namespace sh
{

class VariablePacker
{
  public:
    // Returns true if the passed in variables pack in maxVectors following
    // the packing rules from the GLSL 1.017 spec, Appendix A, section 7.
    // T should be ShaderVariable or one of the subclasses of ShaderVariable.
    template <typename T>
    bool checkVariablesWithinPackingLimits(unsigned int maxVectors,
                                           const std::vector<T> &variables);

    // Gets how many components in a row a data type takes.
    static int GetNumComponentsPerRow(sh::GLenum type);

    // Gets how many rows a data type takes.
    static int GetNumRows(sh::GLenum type);

  private:
    bool checkExpandedVariablesWithinPackingLimits(unsigned int maxVectors,
                                                   std::vector<sh::ShaderVariable> *variables);

    static void ExpandUserDefinedVariable(const ShaderVariable &variable,
                                          const std::string &name,
                                          const std::string &mappedName,
                                          bool markStaticUse,
                                          std::vector<ShaderVariable> *expanded);

    static void ExpandVariable(const ShaderVariable &variable,
                               const std::string &name,
                               const std::string &mappedName,
                               bool markStaticUse,
                               std::vector<ShaderVariable> *expanded);

    static const int kNumColumns      = 4;
    static const unsigned kColumnMask = (1 << kNumColumns) - 1;

    unsigned makeColumnFlags(int column, int numComponentsPerRow);
    void fillColumns(int topRow, int numRows, int column, int numComponentsPerRow);
    bool searchColumn(int column, int numRows, int *destRow, int *destSize);

    int topNonFullRow_;
    int bottomNonFullRow_;
    int maxRows_;
    std::vector<unsigned> rows_;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VARIABLEPACKER_H_
