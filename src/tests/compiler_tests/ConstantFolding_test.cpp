//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConstantFolding_test.cpp:
//   Tests for constant folding
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/PoolAlloc.h"
#include "compiler/translator/TranslatorESSL.h"

#include <vector>

class ConstantFinder : public TIntermTraverser
{
  public:
    ConstantFinder(TConstantUnion constToFind)
        : mConstToFind(constToFind),
          mFound(false),
          mConstantUnionArrayIndex(0)
    {}

    ConstantFinder(TConstantUnion constToFind, unsigned int index)
        : mConstToFind(constToFind),
          mFound(false),
          mConstantUnionArrayIndex(index)
    {}

    virtual void visitConstantUnion(TIntermConstantUnion *node)
    {
        if (node->getUnionArrayPointer()[mConstantUnionArrayIndex] == mConstToFind)
        {
            mFound = true;
        }
    }

    bool found() const { return mFound; }

  private:
    TConstantUnion mConstToFind;
    bool mFound;
    unsigned int mConstantUnionArrayIndex;
};

class ConstantFoldingTest : public testing::Test
{
  public:
    ConstantFoldingTest() {}

  protected:
    virtual void SetUp()
    {
        allocator.push();
        SetGlobalPoolAllocator(&allocator);
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        mTranslatorESSL = new TranslatorESSL(GL_FRAGMENT_SHADER, SH_GLES3_SPEC);
        ASSERT_TRUE(mTranslatorESSL->Init(resources));
    }

    virtual void TearDown()
    {
        delete mTranslatorESSL;
        SetGlobalPoolAllocator(NULL);
        allocator.pop();
    }

    void compile(const std::string& shaderString)
    {
        const char *shaderStrings[] = { shaderString.c_str() };

        mASTRoot = mTranslatorESSL->compileTreeForTesting(shaderStrings, 1, SH_OBJECT_CODE);
        if (!mASTRoot)
        {
            TInfoSink &infoSink = mTranslatorESSL->getInfoSink();
            FAIL() << "Shader compilation into ESSL failed " << infoSink.info.c_str();
        }
    }

    bool constantFoundInAST(TConstantUnion c)
    {
        ConstantFinder finder(c);
        mASTRoot->traverse(&finder);
        return finder.found();
    }

    bool constantFoundInAST(TConstantUnion c, unsigned int index)
    {
        ConstantFinder finder(c, index);
        mASTRoot->traverse(&finder);
        return finder.found();
    }

    bool constantFoundInAST(int i)
    {
        TConstantUnion c;
        c.setIConst(i);
        return constantFoundInAST(c);
    }

    bool constantFoundInAST(float f, unsigned int index)
    {
        TConstantUnion c;
        c.setFConst(f);
        return constantFoundInAST(c, index);
    }

    bool constantFloatVectorFoundInAST(const std::vector<float>& floatVector)
    {
        bool found = true;
        for (size_t i = 0; i < floatVector.size(); i++)
        {
            if (!constantFoundInAST(floatVector[i], i))
            {
                found = false;
                break;
            }
        }
        return found;
    }

  private:
    TranslatorESSL *mTranslatorESSL;
    TIntermNode *mASTRoot;

    TPoolAllocator allocator;
};

TEST_F(ConstantFoldingTest, FoldIntegerAdd)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 + 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    ASSERT_TRUE(constantFoundInAST(1129));
}

TEST_F(ConstantFoldingTest, FoldIntegerSub)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 - 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    ASSERT_TRUE(constantFoundInAST(1119));
}

TEST_F(ConstantFoldingTest, FoldIntegerMul)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 * 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    ASSERT_TRUE(constantFoundInAST(5620));
}

TEST_F(ConstantFoldingTest, FoldIntegerDiv)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 / 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    // Rounding mode of division is undefined in the spec but ANGLE can be expected to round down.
    ASSERT_TRUE(constantFoundInAST(224));
}

TEST_F(ConstantFoldingTest, FoldIntegerModulus)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 % 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    ASSERT_TRUE(constantFoundInAST(4));
}

TEST_F(ConstantFoldingTest, FoldVectorCrossProduct)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec3 my_Vec3;"
        "void main() {\n"
        "   const vec3 v3 = cross(vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, -1.0f, 1.0f));\n"
        "   my_Vec3 = v3;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(constantFoundInAST(1.0f));
    ASSERT_FALSE(constantFoundInAST(-1.0f));
    ASSERT_TRUE(constantFloatVectorFoundInAST(std::vector<float>{2.0f, 0.0f, -2.0f}));
}
