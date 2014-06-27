//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "compiler/translator/VariablePacker.h"
#include "gtest/gtest.h"
#include "angle_gl.h"
#include "common/utilities.h"

static sh::ShaderVariable SimpleVar(GLenum type, unsigned int size)
{
    return sh::ShaderVariable(type, GL_NONE, "", size == 1 ? 0 : size);
}

TEST(VariablePacking, Pack) {
  VariablePacker packer;
  std::vector<sh::ShaderVariable> vars;
  const int kMaxRows = 16;
  // test no vars.
  EXPECT_TRUE(packer.CheckVariablesWithinPackingLimits(kMaxRows, vars));

  sh::GLenum types[] = {
    GL_FLOAT_MAT4,            // 0
    GL_FLOAT_MAT2,            // 1
    GL_FLOAT_VEC4,            // 2
    GL_INT_VEC4,              // 3
    GL_BOOL_VEC4,             // 4
    GL_FLOAT_MAT3,            // 5
    GL_FLOAT_VEC3,            // 6
    GL_INT_VEC3,              // 7
    GL_BOOL_VEC3,             // 8
    GL_FLOAT_VEC2,            // 9
    GL_INT_VEC2,              // 10
    GL_BOOL_VEC2,             // 11
    GL_FLOAT,                 // 12
    GL_INT,                   // 13
    GL_BOOL,                  // 14
    GL_SAMPLER_2D,            // 15
    GL_SAMPLER_CUBE,          // 16
    GL_SAMPLER_EXTERNAL_OES,  // 17
    GL_SAMPLER_2D_RECT_ARB,   // 18
    GL_UNSIGNED_INT,          // 19
    GL_UNSIGNED_INT_VEC2,     // 20
    GL_UNSIGNED_INT_VEC3,     // 21
    GL_UNSIGNED_INT_VEC4,     // 22
    GL_FLOAT_MAT2x3,          // 23
    GL_FLOAT_MAT2x4,          // 24
    GL_FLOAT_MAT3x2,          // 25
    GL_FLOAT_MAT3x4,          // 26
    GL_FLOAT_MAT4x2,          // 27
    GL_FLOAT_MAT4x3,          // 28
    GL_SAMPLER_3D,            // 29
    GL_SAMPLER_2D_ARRAY,      // 30
    GL_SAMPLER_2D_SHADOW,     // 31
    GL_SAMPLER_CUBE_SHADOW,   // 32
    GL_SAMPLER_2D_ARRAY_SHADOW, // 33
    GL_INT_SAMPLER_2D,        // 34
    GL_INT_SAMPLER_CUBE,      // 35
    GL_INT_SAMPLER_3D,        // 36
    GL_INT_SAMPLER_2D_ARRAY,  // 37
    GL_UNSIGNED_INT_SAMPLER_2D, // 38
    GL_UNSIGNED_INT_SAMPLER_CUBE, // 39
    GL_UNSIGNED_INT_SAMPLER_3D, // 40
    GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, // 41
  };

  for (size_t tt = 0; tt < sizeof(types) / sizeof(types[0]); ++tt) {
    sh::GLenum type = types[tt];
    int num_rows = gl::VariableRowCount(type);
    int num_components_per_row = VariablePacker::GetNumComponentsPerRow(type);
    // Check 1 of the type.
    vars.clear();
    vars.push_back(SimpleVar(type, 1));
    EXPECT_TRUE(packer.CheckVariablesWithinPackingLimits(kMaxRows, vars));

    // Check exactly the right amount of 1 type as an array.
    int num_vars = kMaxRows / num_rows;
    vars.clear();
    vars.push_back(SimpleVar(type, num_vars));
    EXPECT_TRUE(packer.CheckVariablesWithinPackingLimits(kMaxRows, vars));

    // test too many
    vars.clear();
    vars.push_back(SimpleVar(type, num_vars + 1));
    EXPECT_FALSE(packer.CheckVariablesWithinPackingLimits(kMaxRows, vars));

    // Check exactly the right amount of 1 type as individual vars.
    num_vars = kMaxRows / num_rows *
        ((num_components_per_row > 2) ? 1 : (4 / num_components_per_row));
    vars.clear();
    for (int ii = 0; ii < num_vars; ++ii) {
      vars.push_back(SimpleVar(type, 1));
    }
    EXPECT_TRUE(packer.CheckVariablesWithinPackingLimits(kMaxRows, vars));

    // Check 1 too many.
    vars.push_back(SimpleVar(type, 1));
    EXPECT_FALSE(packer.CheckVariablesWithinPackingLimits(kMaxRows, vars));
  }

  // Test example from GLSL ES 3.0 spec chapter 11.
  vars.clear();
  vars.push_back(SimpleVar(GL_FLOAT_VEC4, 1));
  vars.push_back(SimpleVar(GL_FLOAT_MAT3, 1));
  vars.push_back(SimpleVar(GL_FLOAT_MAT3, 1));
  vars.push_back(SimpleVar(GL_FLOAT_VEC2, 6));
  vars.push_back(SimpleVar(GL_FLOAT_VEC2, 4));
  vars.push_back(SimpleVar(GL_FLOAT_VEC2, 1));
  vars.push_back(SimpleVar(GL_FLOAT, 3));
  vars.push_back(SimpleVar(GL_FLOAT, 2));
  vars.push_back(SimpleVar(GL_FLOAT, 1));
  EXPECT_TRUE(packer.CheckVariablesWithinPackingLimits(kMaxRows, vars));
}

