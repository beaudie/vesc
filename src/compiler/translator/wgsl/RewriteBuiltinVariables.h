//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_REWRITE_BUILTIN_VARIABLES_H_
#define COMPILER_REWRITE_BUILTIN_VARIABLES_H_

#include "compiler/translator/Common.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/IntermNode.h"

namespace sh
{

// In WGSL, all builtin input values are parameters to the shader's main function and all builtin
// output values are return values of the shader's main functions (the builtins can be embedded
// within struct types). So this rewrites all accesses of GLSL's builtin input variables
// to be accesses of a global struct, so the WGSL traverser can fill in the members of this global
// struct with the appropriate WGSL builtin input variables at the beginning of the main function.
// Similarly all writes to GLSL's builtin output variables are rewritten to be writes to a global
// struct, so the WGSL main function can return that global output struct.
//
// TODO(anglebug.com/42267100): some of these WGSL builtins do not correspond exactly to GLSL
// builtin values and will need modification at the beginning (resp. end) of the main function for
// input values (resp. output values). E.g. normalized device coordinates in GLSL have -1.0 <= z
// <= 1.0, whereas NDC in WGSL have 0.0 <= z <= 1.0.
// See e.g. bool TranslatorMSL::appendVertexShaderDepthCorrectionToMain().
//
// Example GLSL:
//
// void setPosition() {
//   gl_Position = vec4(gl_VertexID, 0.0, 0.0, 1.0);
// }
// void main()
// {
//   setPosition();
// }
//
// PartiallyRewriteBuiltinVariables() will process the GLSL to look like this valid GLSL (which
// contains undefined bahvior, however):
//
// struct ANGLE_Input {
//     vec4 gl_VertexID_;
// };
// Angle_Input ANGLE_input;
// struct ANGLE_Output {
//     vec4 gl_Position_;
// };
// ANGLE_Output ANGLE_output;
// void setPosition() {
//   ANGLE_output.gl_Position_ = vec4(ANGLE_input.gl_VertexID_, 0.0, 0.0, 1.0);
// }
// void main()
// {
//   setPosition();
// }
//
// This would generate this WGSL:
//
// struct ANGLE_Input {
//     gl_VertexID_ : u32;
// };
// var<private> ANGLE_input : ANGLE_Input;
//
// struct ANGLE_Output {
//     gl_Position_: vec4<f32>,
// };
// var<private> ANGLE_output : ANGLE_Output;
//
// fn setPosition() {
//   ANGLE_output.gl_Position_ = vec4<f32>(f32(ANGLE_input.gl_VertexID_), 0.0, 0.0, 1.0);
// }
//
// @vertex
// fn main() {
//     setPosition();
// }
//
// However this is the target WGSL (note the main function changes and the @builtin annotations):
//
// struct ANGLE_Input {
//     @builtin(vertex_index) gl_VertexID_ : u32;
// };
// var<private> ANGLE_input : ANGLE_Input;
//
// struct ANGLE_Output {
//     @builtin(position) gl_Position_: vec4<f32>,
// };
// var<private> ANGLE_output : ANGLE_Output;
//
// fn setPosition() {
//   ANGLE_output.gl_Position = vec4<f32>(f32(ANGLE_input.gl_VertexID), 0.0, 0.0, 1.0);
// }
//
// @vertex
// fn vs_main(
//     ANGLE_Input in_angle_input,
// ) -> ANGLE_Output {
//     ANGLE_input = in_angle_input;
//     setPosition();
//     return ANGLE_vertex_out;
// }
//
// So, note that when outputting WGSL the @builtin() annotations need to be generated even though
// they won't be present in the GLSL AST. Same with the main function parameters/return type, and
// the initialization of ANGLE_input and the return of ANGLE_ouput by the main function.
const char kBuiltinInputStructType[]  = "ANGLE_Input";
const char kBuiltinOutputStructType[] = "ANGLE_Output";
const char kBuiltinInputStructName[]  = "ANGLE_input";
const char kBuiltinOutputStructName[] = "ANGLE_output";

struct AttributeLocationAnnotation
{
    // Most variables will not be assigned a location until link time, but some variables (like
    // gl_FragColor) imply an output location.
    int location = -1;
};
struct BuiltinAnnotation
{
    ImmutableString wgslBuiltinName;
};

using PipelineAnnotation = std::variant<AttributeLocationAnnotation, BuiltinAnnotation>;

using PipelineAnnotationsMap = TMap<const TField *, PipelineAnnotation>;

// `pipelineAnnotationsOut` is an outvariable mapping the fields of ANGLE_Input and ANGLE_Output to
// the correct @builtin() annotations, e.g. pipelineAnnotationsOut[gl_VertexID_field] =
// "vertex_index", to be used by the WGSL generator to annotate struct fields.
[[nodiscard]] bool PartiallyRewriteBuiltinVariables(TCompiler &compiler,
                                                    TIntermBlock &root,
                                                    PipelineAnnotationsMap &pipelineAnnotationsMap,
                                                    bool *needsInputStructOut,
                                                    bool *needsOutputStructOut);

// TODO(mpdenton): comment for new bool fields.

}  // namespace sh

#endif  // COMPILER_REWRITE_BUILTIN_VARIABLES_H_
