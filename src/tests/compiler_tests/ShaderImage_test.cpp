//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
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

// Check that images get properly parsed.
TEST_F(ShaderImageTest, ImageDeclaration)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;"
        "layout(rgba32f) uniform highp readonly image2D myImage;\n"
        "layout(rgba32f) uniform highp writeonly image2D myImage2;\n"
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
    const TIntermSymbol *myImageNode = FindSymbolNode(mASTRoot, "myImage", EbtImage2D);
    ASSERT_NE(nullptr, myImageNode);

    const TType &myImageType                = myImageNode->getType();
    TLayoutQualifier myImageLayoutQualifier = myImageType.getLayoutQualifier();
    ASSERT_EQ(EiifRGBA32F, myImageLayoutQualifier.imageInternalFormat);
    TMemoryQualifier myImageMemoryQualifier = myImageType.getMemoryQualifier();
    ASSERT_TRUE(myImageMemoryQualifier.readonly);
    ASSERT_FALSE(myImageMemoryQualifier.writeonly);

    // get symbol information for myImage2 amd do tests
    const TIntermSymbol *myImage2Node = FindSymbolNode(mASTRoot, "myImage2", EbtImage2D);
    ASSERT_NE(nullptr, myImage2Node);

    const TType &myImage2Type                = myImage2Node->getType();
    TLayoutQualifier myImage2LayoutQualifier = myImage2Type.getLayoutQualifier();
    ASSERT_EQ(EiifRGBA32F, myImage2LayoutQualifier.imageInternalFormat);
    TMemoryQualifier myImage2MemoryQualifier = myImage2Type.getMemoryQualifier();
    ASSERT_TRUE(myImage2MemoryQualifier.writeonly);
    ASSERT_FALSE(myImage2MemoryQualifier.readonly);
}

// Check that imageLoad and imageStore calls get correctly parsed.
TEST_F(ShaderImageTest, ImageStore)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;"
        "layout(rgba32f) uniform highp readonly image2D myImageInput;\n"
        "layout(rgba32f) uniform highp writeonly image2D myImageOutput;\n"
        "void main() {\n"
        "imageStore(myImageOutput, ivec2(gl_LocalInvocationID.xy), vec4(0.0));\n"
        "vec4 result = imageLoad(myImageInput, ivec2(gl_LocalInvocationID.xy) + ivec2(10));\n"
        "}";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed" << mInfoLog;
    }

    // tests for imageStore
    const TIntermAggregate *imageStoreFunctionCall =
        FindFunctionNode(mASTRoot, "imageStore(im21;vi2;vf4;");
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
    const TIntermAggregate *imageLoadFunctionCall =
        FindFunctionNode(mASTRoot, "imageLoad(im21;vi2;");
    ASSERT_NE(nullptr, imageLoadFunctionCall);

    const TIntermSequence *loadArguments = imageLoadFunctionCall->getSequence();
    ASSERT_EQ(2u, loadArguments->size());

    const TIntermTyped *loadArgument1Typed = (*loadArguments)[0]->getAsTyped();
    ASSERT_EQ(EbtImage2D, loadArgument1Typed->getBasicType());

    const TIntermTyped *loadArgument2Typed = (*loadArguments)[1]->getAsTyped();
    ASSERT_EQ(EbtInt, loadArgument2Typed->getBasicType());
    ASSERT_EQ(2, loadArgument2Typed->getNominalSize());
}

// Check that memory qualifiers are correctly parsed.
TEST_F(ShaderImageTest, ImageMemoryQualifiers)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;"
        "layout(rgba32f) uniform highp coherent readonly image2D image1;\n"
        "layout(rgba32f) uniform highp volatile writeonly image2D image2;\n"
        "layout(rgba32f) uniform highp volatile restrict readonly writeonly image2D image3;\n"
        "void main() {\n"
        "}";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed" << mInfoLog;
    }

    const TIntermSymbol *image1Symbol = FindSymbolNode(mASTRoot, "image1", EbtImage2D);
    ASSERT_NE(nullptr, image1Symbol);
    const TMemoryQualifier &image1MemoryQualifier = image1Symbol->getType().getMemoryQualifier();
    ASSERT_EQ(true, image1MemoryQualifier.coherent);
    ASSERT_EQ(true, image1MemoryQualifier.readonly);

    const TIntermSymbol *image2Symbol = FindSymbolNode(mASTRoot, "image2", EbtImage2D);
    ASSERT_NE(nullptr, image2Symbol);
    const TMemoryQualifier &image2MemoryQualifier = image2Symbol->getType().getMemoryQualifier();
    ASSERT_EQ(true, image2MemoryQualifier.coherent);
    ASSERT_EQ(true, image2MemoryQualifier.volatileQualifier);
    ASSERT_EQ(true, image2MemoryQualifier.writeonly);

    const TIntermSymbol *image3Symbol = FindSymbolNode(mASTRoot, "image3", EbtImage2D);
    ASSERT_NE(nullptr, image3Symbol);
    const TMemoryQualifier &image3MemoryQualifier = image3Symbol->getType().getMemoryQualifier();
    ASSERT_EQ(true, image3MemoryQualifier.coherent);
    ASSERT_EQ(true, image3MemoryQualifier.volatileQualifier);
    ASSERT_EQ(true, image3MemoryQualifier.restrictQualifier);
    ASSERT_EQ(true, image3MemoryQualifier.writeonly);
    ASSERT_EQ(true, image3MemoryQualifier.readonly);
}
