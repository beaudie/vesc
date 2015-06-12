//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConstantFolding_test.cpp:
//   Tests for constant folding
//

#include <vector>

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/PoolAlloc.h"
#include "compiler/translator/TranslatorESSL.h"

template <typename T>
class ConstantFinder : public TIntermTraverser
{
  public:
    ConstantFinder(const std::vector<T> &constantVector)
        : TIntermTraverser(true, false, false),
          mConstantVector(constantVector),
          mFound(false)
    {}

    ConstantFinder(const T &value)
        : TIntermTraverser(true, false, false),
          mFound(false)
    {
        mConstantVector.push_back(value);
    }

    void visitConstantUnion(TIntermConstantUnion *node)
    {
        if (node->getType().getObjectSize() == mConstantVector.size())
        {
            bool found = true;
            for (size_t i = 0; i < mConstantVector.size(); i++)
            {
                if (node->getUnionArrayPointer()[i] != mConstantVector[i])
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                mFound = found;
            }
        }
    }

    bool found() const { return mFound; }

  private:
    std::vector<T> mConstantVector;
    bool mFound;
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

    template <typename T>
    bool constantFoundInAST(T constant)
    {
        ConstantFinder<T> finder(constant);
        mASTRoot->traverse(&finder);
        return finder.found();
    }

    template <typename T>
    bool constantVectorFoundInAST(const std::vector<T> &constantVector)
    {
        ConstantFinder<T> finder(constantVector);
        mASTRoot->traverse(&finder);
        return finder.found();
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
    std::vector<float> input1(3, 1.0f);
    ASSERT_FALSE(constantVectorFoundInAST(input1));
    std::vector<float> input2;
    input2.push_back(1.0f);
    input2.push_back(-1.0f);
    input2.push_back(1.0f);
    ASSERT_FALSE(constantVectorFoundInAST(input2));
    std::vector<float> result;
    result.push_back(2.0f);
    result.push_back(0.0f);
    result.push_back(-2.0f);
    ASSERT_TRUE(constantVectorFoundInAST(result));
}

TEST_F(ConstantFoldingTest, FoldMatrixInverse)
{
    const std::string &shaderString1 =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out mat2 my_Matrix;"
        "void main() {\n"
        "   const mat2 m2 = inverse(mat2(2.0f, 3.0f, 5.0f, 7.0f));\n"
        "   my_Matrix = m2;\n"
        "}\n";
    compile(shaderString1);
    float inputElements1[] =
    {
        2.0f, 5.0f,
        3.0f, 7.0f
    };
    std::vector<float> input1(inputElements1, inputElements1 + 4);
    ASSERT_FALSE(constantVectorFoundInAST(input1));
    float outputElements1[] =
    {
        -7.0f, 3.0f,
        5.0f, -2.0f
    };
    std::vector<float> result1(outputElements1, outputElements1+ 4);
    ASSERT_TRUE(constantVectorFoundInAST(result1));

    const std::string &shaderString2 =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out mat3 my_Matrix;"
        "void main() {\n"
        "   const mat3 m3 = inverse(mat3(11.0f, 13.0f, 19.0f,\n"
        "                                23.0f, 29.0f, 31.0f,\n"
        "                                37.0f, 41.0f, 43.0f));\n"
        "   my_Matrix = m3;\n"
        "}\n";
    compile(shaderString2);
    float inputElements2[] =
    {
        11.0f, 23.0f, 37.0f,
        13.0f, 29.0f, 41.0f,
        19.0f, 31.0f, 43.0f
    };
    std::vector<float> input2(inputElements2, inputElements2 + 9);
    ASSERT_FALSE(constantVectorFoundInAST(input2));
    float outputElements2[] =
    {
        3.0f / 85.0f, -79.0f / 340.0f, 13.0f / 68.0f,
        -11.0f / 34.0f, 23.0f / 68.0f, -3.0f / 68.0f,
        37.0f / 170.0f, -12.0f / 85.0f, -1.0f / 34.0f
    };
    std::vector<float> result2(outputElements2, outputElements2 + 9);
    ASSERT_TRUE(constantVectorFoundInAST(result2));

    const std::string &shaderString3 =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out mat4 my_Matrix;"
        "void main() {\n"
        "   const mat4 m4 = inverse(mat4(29.0f, 31.0f, 37.0f, 41.0f,\n"
        "                                43.0f, 47.0f, 53.0f, 59.0f,\n"
        "                                61.0f, 67.0f, 71.0f, 73.0f,\n"
        "                                79.0f, 83.0f, 89.0f, 97.0f));\n"
        "   my_Matrix = m4;\n"
        "}\n";
    compile(shaderString3);
    float inputElements3[] =
    {
        29.0f, 43.0f, 61.0f, 79.0f,
        31.0f, 47.0f, 67.0f, 83.0f,
        37.0f, 53.0f, 71.0f, 89.0f,
        41.0f, 59.0f, 73.0f, 97.0f
    };
    std::vector<float> input3(inputElements3, inputElements3 + 16);
    ASSERT_FALSE(constantVectorFoundInAST(input3));
    float outputElements3[] =
    {
        43.0f / 126.0f, -5.0f / 7.0f, 85.0f / 126.0f, -2.0f / 7.0f,
        -11.0f / 21.0f, 9.0f / 14.0f, -11.0f / 21.0f, 5.0f / 14.0f,
        -2.0f / 21.0f, 1.0f / 14.0f, 43.0f / 210.0f, -6.0f / 35.0f,
        31.0f / 126.0f, -1.0f / 7.0f, -38.0f / 315.0f, 3.0f / 70.0f
    };
    std::vector<float> result3(outputElements3, outputElements3 + 16);
    ASSERT_TRUE(constantVectorFoundInAST(result3));
}

TEST_F(ConstantFoldingTest, FoldMatrixDeterminant)
{
    const std::string &shaderString1 =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out float my_Float;"
        "void main() {\n"
        "   const float f = determinant(mat2(2.0f, 3.0f, 5.0f, 7.0f));\n"
        "   my_Float = f;\n"
        "}\n";
    compile(shaderString1);
    float inputElements1[] =
    {
        2.0f, 5.0f,
        3.0f, 7.0f
    };
    std::vector<float> input1(inputElements1, inputElements1 + 4);
    ASSERT_FALSE(constantVectorFoundInAST(input1));
    ASSERT_TRUE(constantFoundInAST(-1.0f));

    const std::string &shaderString2 =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out float my_Float;"
        "void main() {\n"
        "   const float f = determinant(mat3(11.0f, 13.0f, 19.0f,\n"
        "                               23.0f, 29.0f, 31.0f,\n"
        "                               37.0f, 41.0f, 43.0f));\n"
        "   my_Float = f;\n"
        "}\n";
    compile(shaderString2);
    float inputElements2[] =
    {
        11.0f, 23.0f, 37.0f,
        13.0f, 29.0f, 41.0f,
        19.0f, 31.0f, 43.0f
    };
    std::vector<float> input2(inputElements2, inputElements2 + 9);
    ASSERT_FALSE(constantVectorFoundInAST(input2));
    ASSERT_TRUE(constantFoundInAST(-680.0f));

    const std::string &shaderString3 =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out float my_Float;"
        "void main() {\n"
        "   const float f = determinant(mat4(29.0f, 31.0f, 37.0f, 41.0f,\n"
        "                               43.0f, 47.0f, 53.0f, 59.0f,\n"
        "                               61.0f, 67.0f, 71.0f, 73.0f,\n"
        "                               79.0f, 83.0f, 89.0f, 97.0f));\n"
        "   my_Float = f;\n"
        "}\n";
    compile(shaderString3);
    float inputElements3[] =
    {
        29.0f, 43.0f, 61.0f, 79.0f,
        31.0f, 47.0f, 67.0f, 83.0f,
        37.0f, 53.0f, 71.0f, 89.0f,
        41.0f, 59.0f, 73.0f, 97.0f
    };
    std::vector<float> input3(inputElements3, inputElements3 + 16);
    ASSERT_FALSE(constantVectorFoundInAST(input3));
    ASSERT_TRUE(constantFoundInAST(-2520.0f));
}
