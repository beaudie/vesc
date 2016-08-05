//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderImage_test.cpp:
// Tests for images
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorESSL.h"
#include "tests/test_utils/compiler_test.h"

class ShaderImageTest : public testing::Test
{
  public:
    ShaderImageTest() {}

  protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        mTranslator = new TranslatorESSL(GL_COMPUTE_SHADER, SH_GLES3_1_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    virtual void TearDown() { delete mTranslator; }

    // Return true when compilation succeeds
    bool compile(const std::string &shaderString)
    {
        const char *shaderStrings[] = {shaderString.c_str()};
        mASTRoot                    = mTranslator->compileTreeForTesting(shaderStrings, 1,
                                                      SH_INTERMEDIATE_TREE | SH_VARIABLES);
        TInfoSink &infoSink = mTranslator->getInfoSink();
        mInfoLog            = infoSink.info.c_str();
        return mASTRoot != nullptr;
    }

  protected:
    std::string mTranslatedCode;
    std::string mInfoLog;
    TranslatorESSL *mTranslator;
    TIntermNode *mASTRoot;
};

// check that a simple declaration works
TEST_F(ShaderImageTest, ImageDeclaration)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;"
        "layout(rgba32f) uniform highp image2D myImage;\n"
        "layout(rgba32f) uniform highp image2D myImage2;\n"
        "void doSomething() {}\n"
        "void main() {\n"
        "}";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed" << mInfoLog;
    }

    // check that we have the correct number of uniforms
    const auto &uniforms = mTranslator->getUniforms();
    ASSERT_EQ(2u, uniforms.size());

    const auto &imageUniform = uniforms[0];
    ASSERT_EQ(GL_IMAGE_2D, imageUniform.type);
    ASSERT_STREQ(imageUniform.name.c_str(), "myImage");

    const auto &image2Uniform = uniforms[1];
    ASSERT_EQ(GL_IMAGE_2D, imageUniform.type);
    ASSERT_STREQ(image2Uniform.name.c_str(), "myImage2");

    // get symbol information for myImage amd do tests
    const TIntermSymbol *myImageNode = FindSymbolNode("myImage", EbtImage2D);
    ASSERT_NE(nullptr, myImageNode);

    const TType &myImageType                = myImageNode->getType();
    TLayoutQualifier myImageLayoutQualifier = myImageType.getLayoutQualifier();
    ASSERT_EQ(EiifRGBA32F, myImageLayoutQualifier.imageInternalFormat);

    // get symbol information for myImage2 amd do tests
    const TIntermSymbol *myImage2Node = FindSymbolNode("myImage2", EbtImage2D);
    ASSERT_NE(nullptr, myImage2Node);

    const TType &myImage2Type                = myImage2Node->getType();
    TLayoutQualifier myImage2LayoutQualifier = myImage2Type.getLayoutQualifier();

    ASSERT_EQ(EiifRGBA32F, myImage2LayoutQualifier.imageInternalFormat);
}

// check that imageStore and imageLoad are correct
TEST_F(ShaderImageTest, ImageStore)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;"
        "layout(rgba32f) uniform highp image2D myImageInput;\n"
        "layout(rgba32f) uniform highp image2D myImageOutput;\n"
        "void main() {\n"
        "imageStore(myImageOutput, ivec2(gl_LocalInvocationID.xy), vec4(0.0));\n"
        "vec4 result = imageLoad(myImageInput, ivec2(gl_LocalInvocationID.xy) + ivec2(10));\n"
        "}";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed" << mInfoLog;
    }

    // tests for imageStore
    const TIntermAggregate *imageStoreFunctionCall = FindFunctionNode("imageStore(im21;vi2;vf4;");
    ASSERT_NE(nullptr, imageStoreFunctionCall);

    const TIntermSequence *storeArguments = imageStoreFunctionCall->getSequence();
    ASSERT_EQ(3u, storeArguments->size());

    const TIntermTyped *storeArgument1Typed = (*storeArguments)[0]->getAsTyped();
    ASSERT_EQ(EbtImage2D, storeArgument1Typed->getBasicType());

    const TIntermTyped *storeArgument2Typed = (*storeArguments)[1]->getAsTyped();
    ASSERT_EQ(EbtInt, storeArgument2Typed->getBasicType());
    ASSERT_EQ(2, storeArgument2Typed->getNominalSize());

    const TIntermTyped *storeArgument3Typed = (*storeArguments)[2]->getAsTyped();
    ASSERT_EQ(EbtFloat, storeArgument3Typed->getBasicType());
    ASSERT_EQ(4, storeArgument3Typed->getNominalSize());

    // tests for imageLoad
    const TIntermAggregate *imageLoadFunctionCall = FindFunctionNode("imageLoad(im21;vi2;");
    ASSERT_NE(nullptr, imageLoadFunctionCall);

    const TIntermSequence *loadArguments = imageLoadFunctionCall->getSequence();
    ASSERT_EQ(2u, loadArguments->size());

    const TIntermTyped *loadArgument1Typed = (*loadArguments)[0]->getAsTyped();
    ASSERT_EQ(EbtImage2D, loadArgument1Typed->getBasicType());

    const TIntermTyped *loadArgument2Typed = (*loadArguments)[1]->getAsTyped();
    ASSERT_EQ(EbtInt, loadArgument2Typed->getBasicType());
    ASSERT_EQ(2, loadArgument2Typed->getNominalSize());
}
