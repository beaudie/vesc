//
// Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "angle_gl.h"
#include "compiler/translator/BuiltInFunctionEmulator.h"
#include "compiler/translator/BuiltInFunctionEmulatorGLSL.h"
#include "compiler/translator/SymbolTable.h"

void InitBuiltInFunctionEmulatorForGLSL(BuiltInFunctionEmulator *emu, sh::GLenum shaderType)
{
    // we use macros here instead of function definitions to work around more GLSL
    // compiler bugs, in particular on NVIDIA hardware on Mac OSX. Macros are
    // problematic because if the argument has side-effects they will be repeatedly
    // evaluated. This is unlikely to show up in real shaders, but is something to
    // consider.

    TType float1(EbtFloat);
    TType float2(EbtFloat, 2);
    TType float3(EbtFloat, 3);
    TType float4(EbtFloat, 4);

    if (shaderType == GL_FRAGMENT_SHADER)
    {
        emu->addEmulatedFunction(EOpCos, float1, "webgl_emu_precision float webgl_cos_emu(webgl_emu_precision float a) { return cos(a); }");
        emu->addEmulatedFunction(EOpCos, float2, "webgl_emu_precision vec2 webgl_cos_emu(webgl_emu_precision vec2 a) { return cos(a); }");
        emu->addEmulatedFunction(EOpCos, float3, "webgl_emu_precision vec3 webgl_cos_emu(webgl_emu_precision vec3 a) { return cos(a); }");
        emu->addEmulatedFunction(EOpCos, float4, "webgl_emu_precision vec4 webgl_cos_emu(webgl_emu_precision vec4 a) { return cos(a); }");
    }
    emu->addEmulatedFunction(EOpDistance, float1, float1, "#define webgl_distance_emu(x, y) ((x) >= (y) ? (x) - (y) : (y) - (x))");
    emu->addEmulatedFunction(EOpDot, float1, float1, "#define webgl_dot_emu(x, y) ((x) * (y))");
    emu->addEmulatedFunction(EOpLength, float1, "#define webgl_length_emu(x) ((x) >= 0.0 ? (x) : -(x))");
    emu->addEmulatedFunction(EOpNormalize, float1, "#define webgl_normalize_emu(x) ((x) == 0.0 ? 0.0 : ((x) > 0.0 ? 1.0 : -1.0))");
    emu->addEmulatedFunction(EOpReflect, float1, float1, "#define webgl_reflect_emu(I, N) ((I) - 2.0 * (N) * (I) * (N))");


	//emulate built-in functions missing from OpenGL 4.1
	TType uint1(EbtUInt);
	emu->addEmulatedFunction(EOpPackSnorm2x16, float2,
		"uint webgl_packSnorm2x16_emu(vec2 v){\n"
		"	 int x = int(round(clamp(v.x, -1.0, 1.0) * 32767.0));\n"
		"	 int y = int(round(clamp(v.y, -1.0, 1.0) * 32767.0));\n"
		"	 return (y << 16) | (x & 0xffff);\n"
		"}\n");

	emu->addEmulatedFunction(EOpUnpackSnorm2x16, uint1,
		"float webgl_fromSnorm(uint x){\n"
		"	 int xi = (x & 0x7fff) - (x & 0x8000);\n"
		"	 return clamp(float(xi) / 32767.0, -1.0, 1.0);\n"
		"}\n"
		"vec2 webgl_unpackSnorm2x16_emu(uint u){\n"
		"	 uint y = (u >> 16);\n"
		"	 uint x = u;\n"
		"	 return vec2(webgl_fromSnorm(x), webgl_fromSnorm(y));\n"
		"}\n");

	emu->addEmulatedFunction(EOpPackHalf2x16, float2,
		"uint f32tof16(float val){\n"
		"	 uint f32 = floatBitsToInt(val);\n"
		"	 uint f16 = 0;\n"
		"	 int sign = (f32 >> 16) & 0x8000;\n"
		"	 int exponent = ((f32 >> 23) & 0xff) - 127;\n"
		"	 int mantissa = f32 & 0x007fffff;\n"
		"	 if (exponent == 128) { f16 = sign | (0x1F << 10); if (mantissa) f16 |= (mantissa & 0x3ff); }\n"
		"	 else if (exponent > 15) { f16 = sign | (0x1F << 10); }\n"
		"	 else if (exponent > -15) { exponent += 15; mantissa >>= 13; f16 = sign | exponent << 10 | mantissa; }\n"
		"	 else { f16 = sign; }\n"
		"	 return f16;\n"
		"}\n"
		"uint webgl_packHalf2x16_emu(vec2 v){\n"
		"	 uint x = f32tof16(v.x);\n"
		"	 uint y = f32tof16(v.y);\n"
		"	 return (y << 16) | x;\n"
		"}\n");

	emu->addEmulatedFunction(EOpUnpackHalf2x16, uint1,
		"float f16tof32(uint val){\n"
		"	 int sign = (val & 0x8000) << 15;\n"
		"	 int exponent = (val & 0x7c00) >> 10;\n"
		"	 int mantissa = (val & 0x03ff);\n"
		"	 float f32 = 0.0;\n"
		"	 if (exponent == 0) { if(mantissa != 0){ f32 = (1.0 / (1 << 24)) * mantissa; } }\n"
		"	 else if (exponent == 31) { f32 = intBitsToFloat(sign | 0x7f800000 | mantissa); }\n"
		"	 else {\n"
		"		float scale, decimal;\n"
		"		exponent -= 15;\n"
		"		if (exponent < 0) { scale = 1.0 / (1 << -exponent); }\n"
		"	    else { scale = 1 << exponent; }\n"
		"	    decimal = 1.0 + intBitsToFloat(mantissa / (1 << 10));\n"
		"	    f32 = scale * decimal;\n"
		"	 }\n"
		"	 if (sign) f32 = -f32;\n"
		"	 return f32;\n"
		"}\n"
		"vec2 webgl_unpackHalf2x16_emu(uint u){\n"
		"	 uint y = (u >> 16);\n"
		"	 uint x = u & 0xffff;\n"
		"	 return vec2(f16tof32(x), f16tof32(y));\n"
		"}\n");

}
