//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/TranslatorGLSL.h"

#include "compiler/translator/EmulateCompoundAssignment.h"
#include "compiler/translator/OutputGLSL.h"
#include "compiler/translator/VersionGLSL.h"


namespace {

static void writeVectorPrecisionEmulationHelpers(TInfoSinkBase& sink, unsigned int size) {
    sink <<
    "vec" << size << " webgl_frm(in vec" << size << " v) {\n"
    "    v = clamp(v, -65504.0, 65504.0);\n"
    "    vec" << size << " exponent = floor(log2(abs(v) + 1e-30)) - 10.0;\n"
    "    bvec" << size << " isNonZero = greaterThanEqual(exponent, vec" << size << "(-25.0));\n"
    "    v = v * exp2(-exponent);\n"
    "    v = sign(v) * floor(abs(v));\n"
    "    return v * exp2(exponent) * vec" << size << "(isNonZero);\n"
    "}\n"

    "vec" << size << " webgl_frl(in vec" << size << " v) {\n"
    "    v = clamp(v, -2.0, 2.0);\n"
    "    v = v * 256.0;\n"
    "    v = sign(v) * floor(abs(v));\n"
    "    return v * 0.00390625;\n"
    "}\n";
}

static void writeMatrixPrecisionEmulationHelpers(TInfoSinkBase& sink, const char *functionName) {
    sink <<
    "mat2 " << functionName << "(in mat2 m) {\n"
    "    mat2 rounded;\n"
    "    rounded[0] = " << functionName << "(m[0]);\n"
    "    rounded[1] = " << functionName << "(m[1]);\n"
    "    return rounded;\n"
    "}\n"

    "mat3 " << functionName << "(in mat3 m) {\n"
    "    mat3 rounded;\n"
    "    rounded[0] = " << functionName << "(m[0]);\n"
    "    rounded[1] = " << functionName << "(m[1]);\n"
    "    rounded[2] = " << functionName << "(m[2]);\n"
    "    return rounded;\n"
    "}\n"

    "mat4 " << functionName << "(in mat4 m) {\n"
    "    mat4 rounded;\n"
    "    rounded[0] = " << functionName << "(m[0]);\n"
    "    rounded[1] = " << functionName << "(m[1]);\n"
    "    rounded[2] = " << functionName << "(m[2]);\n"
    "    rounded[3] = " << functionName << "(m[3]);\n"
    "    return rounded;\n"
    "}\n";
}

static void writePrecisionEmulationHelpers(TInfoSinkBase& sink) {
    // Write the webgl_frm functions that round floating point numbers to
    // half precision, and webgl_frl functions that round them to minimum lowp
    // precision.

    // Unoptimized version of webgl_frm for single floats:
    //
    // int webgl_maxNormalExponent(in int exponentBits) {
    //     int possibleExponents = int(exp2(float(exponentBits)));
    //     int exponentBias = possibleExponents / 2 - 1;
    //     int allExponentBitsOne = possibleExponents - 1;
    //     return (allExponentBitsOne - 1) - exponentBias;
    // }
    //
    // float webgl_frm(in float x) {
    //     int mantissaBits = 10;
    //     int exponentBits = 5;
    //     float possibleMantissas = exp2(float(mantissaBits));
    //     float mantissaMax = 2.0 - 1.0 / possibleMantissas;
    //     int maxNE = webgl_maxNormalExponent(exponentBits);
    //     float max = exp2(float(maxNE)) * mantissaMax;
    //     if (x > max) {
    //         return max;
    //     }
    //     if (x < -max) {
    //         return -max;
    //     }
    //     float exponent = floor(log2(abs(x)));
    //     if (abs(x) == 0.0 || exponent < -float(maxNE)) {
    //         return 0.0 * sign(x)
    //     }
    //     x = x * exp2(-(exponent - float(mantissaBits)));
    //     x = sign(x) * floor(abs(x));
    //     return x * exp2(exponent - float(mantissaBits));
    // }

    // All numbers with a magnitude less than 2^-15 are subnormal, and are
    // flushed to zero.

    // Note the constant numbers below:
    // a) 65504 is the maximum possible mantissa (1.1111111111 in binary) times
    //    2^15, the maximum normal exponent.
    // b) 10.0 is the number of mantissa bits.
    // c) -25.0 is the minimum normal half-float exponent -15.0 minus the number
    //    of mantissa bits.
    // d) + 1e-30 is to make sure the argument of log2() won't be zero. It can
    //    only affect the result of log2 on x where abs(x) < 1e-22. Since these
    //    numbers will be flushed to zero either way (2^-15 is the smallest
    //    normal positive number), this does not introduce any error.

    sink <<
    "float webgl_frm(in float x) {\n"
    "    x = clamp(x, -65504.0, 65504.0);\n"
    "    float exponent = floor(log2(abs(x) + 1e-30)) - 10.0;\n"
    "    bool isNonZero = (exponent >= -25.0);\n"
    "    x = x * exp2(-exponent);\n"
    "    x = sign(x) * floor(abs(x));\n"
    "    return x * exp2(exponent) * float(isNonZero);\n"
    "}\n"

    "float webgl_frl(in float x) {\n"
    "    x = clamp(x, -2.0, 2.0);\n"
    "    x = x * 256.0;\n"
    "    x = sign(x) * floor(abs(x));\n"
    "    return x * 0.00390625;\n"
    "}\n";

    writeVectorPrecisionEmulationHelpers(sink, 2);
    writeVectorPrecisionEmulationHelpers(sink, 3);
    writeVectorPrecisionEmulationHelpers(sink, 4);
    writeMatrixPrecisionEmulationHelpers(sink, "webgl_frm");
    writeMatrixPrecisionEmulationHelpers(sink, "webgl_frl");
}

} // namespace anonymous


TranslatorGLSL::TranslatorGLSL(sh::GLenum type, ShShaderSpec spec)
    : TCompiler(type, spec, SH_GLSL_OUTPUT) {
}

void TranslatorGLSL::translate(TIntermNode* root) {
    TInfoSinkBase& sink = getInfoSink().obj;

    // Write GLSL version.
    writeVersion(root);

    writePragma();

    // Write extension behaviour as needed
    writeExtensionBehavior();

    bool precisionEmulation = getResources().WEBGL_debug_shader_precision && getPragma().debugShaderPrecision;

    if (precisionEmulation)
    {
        writePrecisionEmulationHelpers(sink);
        EmulateCompoundAssignment emulateCompoundAssignment;
        root->traverse(&emulateCompoundAssignment);
        emulateCompoundAssignment.writeEmulation(sink);
    }

    // Write emulated built-in functions if needed.
    getBuiltInFunctionEmulator().OutputEmulatedFunctionDefinition(
        sink, false);

    // Write array bounds clamping emulation if needed.
    getArrayBoundsClamper().OutputClampingFunctionDefinition(sink);

    // Write translated shader.
    TOutputGLSL outputGLSL(sink, getArrayIndexClampingStrategy(), getHashFunction(), getNameMap(), getSymbolTable(), getShaderVersion(), precisionEmulation);
    root->traverse(&outputGLSL);
}

void TranslatorGLSL::writeVersion(TIntermNode *root)
{
    TVersionGLSL versionGLSL(getShaderType(), getPragma());
    root->traverse(&versionGLSL);
    int version = versionGLSL.getVersion();
    // We need to write version directive only if it is greater than 110.
    // If there is no version directive in the shader, 110 is implied.
    if (version > 110)
    {
        TInfoSinkBase& sink = getInfoSink().obj;
        sink << "#version " << version << "\n";
    }
}

void TranslatorGLSL::writeExtensionBehavior() {
    TInfoSinkBase& sink = getInfoSink().obj;
    const TExtensionBehavior& extensionBehavior = getExtensionBehavior();
    for (TExtensionBehavior::const_iterator iter = extensionBehavior.begin();
         iter != extensionBehavior.end(); ++iter) {
        if (iter->second == EBhUndefined)
            continue;

        // For GLSL output, we don't need to emit most extensions explicitly,
        // but some we need to translate.
        if (iter->first == "GL_EXT_shader_texture_lod") {
            sink << "#extension GL_ARB_shader_texture_lod : "
                 << getBehaviorString(iter->second) << "\n";
        }
    }
}
