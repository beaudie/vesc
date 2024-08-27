//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_REWRITE_BUILTIN_VARIABLES_H_
#define COMPILER_REWRITE_BUILTIN_VARIABLES_H_

#include <variant>

#include "compiler/translator/Common.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/IntermNode.h"

namespace sh
{

// In WGSL, all input values are parameters to the shader's main function and all output values are
// return values of the shader's main functions (the input/output values can be embedded within
// struct types). So this rewrites all accesses of GLSL's input/output variables (including
// builtins) to be accesses of a global struct, so the WGSL traverser can fill in the members of
// this global struct with the appropriate WGSL input variables at the beginning of the main
// function. Similarly all writes to GLSL's output variables are rewritten to be writes to a global
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
// attribute vec2 xy_position;
// void setPosition() {
//   gl_Position = vec4(xy_position.x, xy_position.y, gl_VertexID, 1.0);
// }
// void main()
// {
//   setPosition();
// }
//
// PartiallyRewritePipelineVariables() will process the GLSL to look like this valid GLSL (which
// contains undefined bahvior, however):
//
// struct ANGLE_Input {
//     int gl_VertexID_;
//     vec2 xy_position;
// };
// Angle_Input ANGLE_input;
// struct ANGLE_Output {
//     vec4 gl_Position_;
// };
// ANGLE_Output ANGLE_output;
// void setPosition() {
//   ANGLE_output.gl_Position_ = vec4(ANGLE_input.xy_position.x, ANGLE_input.xy_position.y,
//     ANGLE_input.gl_VertexID_, 1.0);
// }
// void main()
// {
//   setPosition();
// }
//
// This would generate this WGSL:
//
// struct ANGLE_Input {
//     gl_VertexID_ : i32;
//     vec2<f32> xy_position;
// };
// var<private> ANGLE_input : ANGLE_Input;
//
// struct ANGLE_Output {
//     gl_Position_: vec4<f32>,
// };
// var<private> ANGLE_output : ANGLE_Output;
//
// fn setPosition() {
//   ANGLE_output.gl_Position_ = vec4<f32>(ANGLE_input.xy_position.x, ANGLE_input.xy_position.y,
//     f32(ANGLE_input.gl_VertexID_), 1.0);
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
//     @location(0) xy_position : vec2<f32>;
// };
// var<private> ANGLE_input : ANGLE_Input;
//
// struct ANGLE_Output {
//     @builtin(position) gl_Position_: vec4<f32>,
// };
// var<private> ANGLE_output : ANGLE_Output;
//
// fn setPosition() {
//   ANGLE_output.gl_Position_ = vec4<f32>(ANGLE_input.xy_position.x, ANGLE_input.xy_position.y,
//     f32(ANGLE_input.gl_VertexID_), 1.0);
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
// So, note that when outputting WGSL the @builtin() and @location() annotations need to be
// generated even though they won't be present in the GLSL AST. Same with the main function
// parameters/return type, and the initialization of ANGLE_input and the return of ANGLE_ouput by
// the main function.
const char kBuiltinInputStructType[]  = "ANGLE_Input";
const char kBuiltinOutputStructType[] = "ANGLE_Output";
const char kBuiltinInputStructName[]  = "ANGLE_input";
const char kBuiltinOutputStructName[] = "ANGLE_output";

struct LocationAnnotation
{
    // Most variables will not be assigned a location until link time, but some variables (like
    // gl_FragColor) imply an output location.
    int location = -1;
};
struct BuiltinAnnotation
{
    ImmutableString wgslBuiltinName;
};

using PipelineAnnotation = std::variant<LocationAnnotation, BuiltinAnnotation>;

using PipelineAnnotationsMap = TMap<const TField *, PipelineAnnotation>;

// `pipelineAnnotationsOut` is an outvariable mapping the fields of ANGLE_Input and ANGLE_Output to
// the correct @builtin() and @location annotations, e.g. pipelineAnnotationsOut[gl_VertexID_field]
// = BuiltinAnnotation{"vertex_index"}, to be used by the WGSL generator to annotate struct fields.
// Since WGSL does not allow empty structs, outvariables `needsInputStructOut` and
// `needsOutputStructOut` indicate whether the structs should be used or not.
// TODO(anglebug.com/42267100): collect outvariables into a struct.
[[nodiscard]] bool PartiallyRewritePipelineVariables(TCompiler &compiler,
                                                     TIntermBlock &root,
                                                     PipelineAnnotationsMap &pipelineAnnotationsMap,
                                                     bool *needsInputStructOut,
                                                     bool *needsOutputStructOut);

}  // namespace sh

#endif  // COMPILER_REWRITE_BUILTIN_VARIABLES_H_
