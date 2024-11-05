//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SeparateCompoundExpressions.cpp:
//   Tests that compound expressions are rewritten to simple if statements.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{
#if ANGLE_ENABLE_METAL

class SeparateCompoundExpressions : public MatchOutputCodeTest
{
  public:
    SeparateCompoundExpressions() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions            = {};
        defaultCompileOptions.validateAST                 = true;
        defaultCompileOptions.separateCompoundExpressions = true;
        setDefaultCompileOptions(defaultCompileOptions);
    }
};

TEST_F(SeparateCompoundExpressions, IfAnd)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
uniform bool d0;
uniform bool d1;
out vec4 o;
void main() {
    if (d0 && d1)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
uniform bool _ud0;
uniform bool _ud1;
out highp vec4 _uo;
void main(){
  bool sbbd = _ud0;
  if (sbbd)
  {
    (sbbd = _ud1);
  }
  if (sbbd)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, IfOr)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
uniform bool d0;
uniform bool d1;
out vec4 o;
void main() {
    if (d0 || d1)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
uniform bool _ud0;
uniform bool _ud1;
out highp vec4 _uo;
void main(){
  bool sbbd = _ud0;
  if ((!sbbd))
  {
    (sbbd = _ud1);
  }
  if (sbbd)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, IfAndOr)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
uniform bool d0;
uniform bool d1;
uniform bool d2;
out vec4 o;
void main() {
    if (d0 && d1 || d2)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
uniform bool _ud0;
uniform bool _ud1;
uniform bool _ud2;
out highp vec4 _uo;
void main(){
  bool sbbe = _ud0;
  if (sbbe)
  {
    (sbbe = _ud1);
  }
  bool sbbf = sbbe;
  if ((!sbbf))
  {
    (sbbf = _ud2);
  }
  if (sbbf)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, IfAndOr2)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
uniform bool d0;
uniform bool d1;
uniform bool d2;
out vec4 o;
void main() {
    if (d0 && (d1 || d2))
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
uniform bool _ud0;
uniform bool _ud1;
uniform bool _ud2;
out highp vec4 _uo;
void main(){
  bool sbbf = _ud0;
  if (sbbf)
  {
    bool sbbe = _ud1;
    if ((!sbbe))
    {
      (sbbe = _ud2);
    }
    (sbbf = sbbe);
  }
  if (sbbf)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, IfIntSubExpr)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
uniform int d0;
uniform bool d1;
out vec4 o;
void main() {
    if (2*d0+1 < 0 && d1)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
uniform mediump int _ud0;
uniform bool _ud1;
out highp vec4 _uo;
void main(){
  const mediump int sbc1 = 2;
  mediump int sbbd = (sbc1 * _ud0);
  const mediump int sbc2 = 1;
  mediump int sbbe = (sbbd + sbc2);
  bool sbc0 = (sbbe < 0);
  if (sbc0)
  {
    (sbc0 = _ud1);
  }
  if (sbc0)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, IfAnds)
{
    const char kShader[] = R"(#version 300 es
precision highp float;
uniform bool d0;
uniform bool d1;
uniform bool d2;
out vec4 o;
void main() {
    if ((d0 && d1 && d2) && (d0 && d1 && d2) && (d0 && d1 && d2))
        o = vec4(1);
})";

    const char kExpected[] = R"(#version 300 es
uniform bool _ud0;
uniform bool _ud1;
uniform bool _ud2;
out highp vec4 _uo;
void main(){
  bool sbc5 = _ud0;
  if (sbc5)
  {
    bool sbc4 = _ud1;
    if (sbc4)
    {
      bool sbc3 = _ud2;
      if (sbc3)
      {
        bool sbc2 = _ud0;
        if (sbc2)
        {
          bool sbc1 = _ud1;
          if (sbc1)
          {
            bool sbc0 = _ud2;
            if (sbc0)
            {
              bool sbbf = _ud0;
              if (sbbf)
              {
                bool sbbe = _ud1;
                if (sbbe)
                {
                  (sbbe = _ud2);
                }
                (sbbf = sbbe);
              }
              (sbc0 = sbbf);
            }
            (sbc1 = sbc0);
          }
          (sbc2 = sbc1);
        }
        (sbc3 = sbc2);
      }
      (sbc4 = sbc3);
    }
    (sbc5 = sbc4);
  }
  if (sbc5)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, AssignmentBitOrAnd)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
uniform int d0;
uniform int d1;
uniform int d2;
out vec4 o;
void main() {
    int b = d0 | d1 & d2;
    if (b != 0)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
uniform mediump int _ud0;
uniform mediump int _ud1;
uniform mediump int _ud2;
out highp vec4 _uo;
void main(){
  mediump int sbbf = (_ud1 & _ud2);
  mediump int _ub = (_ud0 | sbbf);
  bool sbc1 = (_ub != 0);
  if (sbc1)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, AssignmentBitOrAnd2)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
uniform int d0;
uniform int d1;
uniform int d2;
uniform int d3;
out vec4 o;
void main() {
    int b = d0 | d1 & d2 & d3;
    if (b != 0)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
uniform mediump int _ud0;
uniform mediump int _ud1;
uniform mediump int _ud2;
uniform mediump int _ud3;
out highp vec4 _uo;
void main(){
  mediump int sbc0 = (_ud1 & _ud2);
  mediump int sbc1 = (sbc0 & _ud3);
  mediump int _ub = (_ud0 | sbc1);
  bool sbc3 = (_ub != 0);
  if (sbc3)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, AssignmentFunc)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
uniform int d0;
int g;
int f(int a) {
  int b = g;
  g = a;
  return a;
}
out vec4 o;
void main() {
    int b = f(1) & (f(2) & f(3));
    if (b != 0)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
uniform mediump int _ud0;
mediump int _ug;
mediump int _uf(in mediump int _ua){
  (_ug = _ua);
  return _ua;
}
out highp vec4 _uo;
void main(){
  mediump int sbc1 = _uf(1);
  mediump int sbc2 = _uf(2);
  mediump int sbc3 = _uf(3);
  mediump int sbc4 = (sbc2 & sbc3);
  mediump int _ub = (sbc1 & sbc4);
  bool sbc6 = (_ub != 0);
  if (sbc6)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, AssignmentCommaAnd)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
uniform bool d0;
uniform bool d1;
uniform bool d2;
out vec4 o;
void main() {
    bool b;
    b = d0, d1 && d2;
    if (b)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
uniform bool _ud0;
uniform bool _ud1;
uniform bool _ud2;
out highp vec4 _uo;
void main(){
  bool _ub;
  bool sbbf = (_ub = _ud0);
  bool sbc0 = _ud1;
  if (sbc0)
  {
    (sbc0 = _ud2);
  }
  sbc0;
  if (_ub)
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundExpressions, ForAndOr)
{
    const char kShader[] = R"(#version 300 es
precision highp float;
uniform int d0;
uniform bool d1;
out vec4 o;
void main() {
    for (int i = 0, b = d0; i < 100 && b > 60 || d1; ++i)
        o = vec4(i);
})";

    const char kExpected[] = R"(#version 300 es
uniform mediump int _ud0;
uniform bool _ud1;
out highp vec4 _uo;
void main(){
  {
    mediump int _ui = 0;
    mediump int _ub = _ud0;
    bool sbc2 = (_ui < 100);
    if (sbc2)
    {
      (sbc2 = (_ub > 60));
    }
    bool sbc3 = sbc2;
    if ((!sbc3))
    {
      (sbc3 = _ud1);
    }
    bool sbbf = sbc3;
    while (sbbf)
    {
      {
        (_uo = vec4(_ui));
      }
      (++_ui);
      bool sbc7 = (_ui < 100);
      if (sbc7)
      {
        (sbc7 = (_ub > 60));
      }
      bool sbc8 = sbc7;
      if ((!sbc8))
      {
        (sbc8 = _ud1);
      }
      (sbbf = sbc8);
    }
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

#endif

}  // namespace
