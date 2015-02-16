#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

class GLES3BuiltInCommonFunctionsTests : public testing::Test
{
public:
    GLES3BuiltInCommonFunctionsTests() {}
protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        mVShadercompiler = ShConstructCompiler(GL_VERTEX_SHADER, SH_GLES3_SPEC, SH_HLSL_OUTPUT, &resources);
        EXPECT_NE(static_cast<ShHandle>(0), mVShadercompiler);

        mFShadercompiler = ShConstructCompiler(GL_FRAGMENT_SHADER, SH_GLES3_SPEC, SH_HLSL_OUTPUT, &resources);
        EXPECT_NE(static_cast<ShHandle>(0), mFShadercompiler);
    }

    virtual void TearDown()
    {
        ShDestruct(mVShadercompiler);
        ShDestruct(mFShadercompiler);
    }

    ShHandle mVShadercompiler;
    ShHandle mFShadercompiler;
};

TEST_F(GLES3BuiltInCommonFunctionsTests, max)
{
    // Check if max(genIType x, int y) and max(genIType x, genIType y) compiles successfully.
    const std::string &shader1 =
        "#version 300 es\n"
        "void main() {\n"

        "int max_int_int_val = max(int(-10), int(-12)); \n"
        "ivec2 max_ivec2_int_val = max(ivec2(int(-10), int(-12)), int(-12)); \n"
        "ivec3 max_ivec3_int_val = max(ivec3(int(-10), int(-12), int(-14)), int(-12)); \n"
        "ivec4 max_ivec4_int_val = max(ivec4(int(-10), int(-12), int(-14), int(-14)), int(-12)); \n"
        "ivec2 max_ivec2_ivec2_val = max(ivec2(int(-10), int(-12)), ivec2(int(-12), int(-14))); \n"
        "ivec3 max_ivec3_ivec3_val = max(ivec3(int(-10), int(-12), int(-14)), ivec3(int(-12), int(-14), int(-16))); \n"
        "ivec4 max_ivec4_ivec4_val = max(ivec4(int(-10), int(-12), int(-14), int(-16)), ivec4(int(-12), int(-14), int(-16), int(-18))); \n"

        "}\n";
    const char *shader1Strings[] = { shader1.c_str() };

    EXPECT_TRUE(ShCompile(mVShadercompiler, shader1Strings, 1, SH_OBJECT_CODE));
    EXPECT_TRUE(ShCompile(mFShadercompiler, shader1Strings, 1, SH_OBJECT_CODE));

    // Check if max(genUType x, uint y) and max(genUType x, genUType y) cimpiles successfully.
    const std::string &shader2 =
        "#version 300 es\n"
        "void main() {\n"

        "uint max_uint_uint_val = max(uint(10), uint(12)); \n"
        "uvec2 max_uvec2_uint_val = max(uvec2(int(10), uint(12)), uint(12)); \n"
        "uvec3 max_uvec3_uint_val = max(uvec3(uint(10), uint(-12), uint(14)), uint(12)); \n"
        "uvec4 max_uvec4_uint_val = max(uvec4(uint(10), uint(12), uint(14), uint(14)), uint(12)); \n"
        "uvec2 max_uvec2_uvec2_val = max(uvec2(uint(10), uint(12)), uvec2(uint(12), uint(14))); \n"
        "uvec3 max_uvec3_uvec3_val = max(uvec3(uint(10), uint(12), uint(14)), uvec3(uint(12), uint(14), uint(16))); \n"
        "uvec4 max_uvec4_uvec4_val = max(uvec4(uint(10), uint(12), uint(14), uint(16)), uvec4(int(12), uint(14), uint(16), uint(18))); \n"

        "}\n";
    const char *shader2Strings[] = { shader2.c_str() };

    EXPECT_TRUE(ShCompile(mVShadercompiler, shader2Strings, 1, SH_OBJECT_CODE));
    EXPECT_TRUE(ShCompile(mFShadercompiler, shader2Strings, 1, SH_OBJECT_CODE));
}

TEST_F(GLES3BuiltInCommonFunctionsTests, min)
{
    // Check if min(genIType x, int y) and min(genIType x, genIType y) compiles successfully.
    const std::string &shader1 =
        "#version 300 es\n"
        "void main() {\n"

        "int min_int_int_val = max(int(-10), int(-12)); \n"
        "ivec2 min_ivec2_int_val = min(ivec2(int(-10), int(-12)), int(-12)); \n"
        "ivec3 min_ivec3_int_val = min(ivec3(int(-10), int(-12), int(-14)), int(-12)); \n"
        "ivec4 min_ivec4_int_val = min(ivec4(int(-10), int(-12), int(-14), int(-14)), int(-12)); \n"
        "ivec2 min_ivec2_ivec2_val = min(ivec2(int(-10), int(-12)), ivec2(int(-12), int(-14))); \n"
        "ivec3 min_ivec3_ivec3_val = min(ivec3(int(-10), int(-12), int(-14)), ivec3(int(-12), int(-14), int(-16))); \n"
        "ivec4 min_ivec4_ivec4_val = min(ivec4(int(-10), int(-12), int(-14), int(-16)), ivec4(int(-12), int(-14), int(-16), int(-18))); \n"

        "}\n";
    const char *shader1Strings[] = { shader1.c_str() };

    EXPECT_TRUE(ShCompile(mVShadercompiler, shader1Strings, 1, SH_OBJECT_CODE));
    EXPECT_TRUE(ShCompile(mFShadercompiler, shader1Strings, 1, SH_OBJECT_CODE));

    // Check if min(genUType x, uint y) and min(genUType x, genUType y) compiles successfully.
    const std::string &shader2 =
        "#version 300 es\n"
        "void main() {\n"

        "uint min_uint_uint_val = min(uint(10), uint(12)); \n"
        "uvec2 min_uvec2_uint_val = min(uvec2(int(10), uint(12)), uint(12)); \n"
        "uvec3 min_uvec3_uint_val = min(uvec3(uint(10), uint(-12), uint(14)), uint(12)); \n"
        "uvec4 min_uvec4_uint_val = min(uvec4(uint(10), uint(12), uint(14), uint(14)), uint(12)); \n"
        "uvec2 min_uvec2_uvec2_val = min(uvec2(uint(10), uint(12)), uvec2(uint(12), uint(14))); \n"
        "uvec3 min_uvec3_uvec3_val = min(uvec3(uint(10), uint(12), uint(14)), uvec3(uint(12), uint(14), uint(16))); \n"
        "uvec4 min_uvec4_uvec4_val = min(uvec4(uint(10), uint(12), uint(14), uint(16)), uvec4(int(12), uint(14), uint(16), uint(18))); \n"

        "}\n";
    const char *shader2Strings[] = { shader2.c_str() };

    EXPECT_TRUE(ShCompile(mVShadercompiler, shader2Strings, 1, SH_OBJECT_CODE));
    EXPECT_TRUE(ShCompile(mFShadercompiler, shader2Strings, 1, SH_OBJECT_CODE));
}

TEST_F(GLES3BuiltInCommonFunctionsTests, clamp)
{
    // Check if clamp(genIType x, int minVal, int maxVal) and clamp(genIType x, genIType minVal, genIType maxVal) compiles successfully.
    const std::string &shader1 =
        "#version 300 es\n"
        "void main() {\n"

        "int clamp_int_int_int_val = clamp(int(-10), int(0), int(10)); \n"
        "ivec2 clamp_ivec2_int_int_val = clamp(ivec2(int(-10), int(10)), int(0), int(10)); \n"
        "ivec3 clamp_ivec3_int_int_val = clamp(ivec3(int(-10), int(0), int(10)), int(0), int(10)); \n"
        "ivec4 clamp_ivec4_int_int_val = clamp(ivec4(int(-10), int(0), int(10), int(20)), int(0), int(10)); \n"
        "ivec2 clamp_ivec2_ivec2_ivec2_val = clamp(ivec2(int(-10), int(0)), ivec2(int(0), int(0)), ivec2(int(10), int(10))); \n"
        "ivec3 clamp_ivec3_ivec3_ivec3_val = clamp(ivec3(int(-10), int(0), int(10)), ivec3(int(0), int(0), int(0)), ivec3(int(10), int(10), int(10))); \n"
        "ivec4 clamp_ivec4_ivec4_ivec4_val = clamp(ivec4(int(-10), int(0), int(10), int(20)), ivec4(int(0), int(0), int(0), int(0)), ivec4(int(10), int(10), int(10), int(10))); \n"

        "}\n";
    const char *shader1Strings[] = { shader1.c_str() };

    EXPECT_TRUE(ShCompile(mVShadercompiler, shader1Strings, 1, SH_OBJECT_CODE));
    EXPECT_TRUE(ShCompile(mFShadercompiler, shader1Strings, 1, SH_OBJECT_CODE));

    // Check if clamp (genUType x, genUType minVal, genUType maxVal) and clamp(genUType x, uint minVal, uint maxVal) compiles successfully.
    const std::string &shader2 =
        "#version 300 es\n"
        "void main() {\n"

        "uint clamp_uint_uint_uint_val = clamp(uint(10), uint(0), uint(10)); \n"
        "uvec2 clamp_uvec2_uint_uint_val = clamp(uvec2(uint(10), uint(20)), uint(0), uint(10)); \n"
        "uvec3 clamp_uvec3_uint_uint_val = clamp(uvec3(uint(10), uint(20), uint(30)), uint(0), uint(10)); \n"
        "uvec4 clamp_uvec4_uint_uint_val = clamp(uvec4(uint(10), uint(20), uint(30), uint(40)), uint(0), uint(10)); \n"
        "uvec2 clamp_uvec2_uvec2_uvec2_val = clamp(uvec2(uint(10), uint(20)), uvec2(uint(0), uint(0)), uvec2(uint(10), uint(10))); \n"
        "uvec3 clamp_uvec3_uvec3_uvec3_val = clamp(uvec3(uint(10), uint(20), uint(30)), uvec3(uint(0), uint(0), uint(0)), uvec3(uint(10), uint(10), uint(10))); \n"
        "uvec4 clamp_uvec4_uvec4_uvec4_val = clamp(uvec4(uint(10), uint(20), uint(30), uint(40)), uvec4(uint(0), uint(0), uint(0), uint(0)), uvec4(uint(10), uint(10), uint(10), uint(10))); \n"

        "}\n";
    const char *shader2Strings[] = { shader2.c_str() };

    EXPECT_TRUE(ShCompile(mVShadercompiler, shader2Strings, 1, SH_OBJECT_CODE));
    EXPECT_TRUE(ShCompile(mFShadercompiler, shader2Strings, 1, SH_OBJECT_CODE));
}