//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// matrix_utils_unittests:
//   Unit tests for the matrix utils.
//

#include "matrix_utils.h"

#include <gtest/gtest.h>

using namespace angle;

namespace
{

const unsigned int minDimensions = 2;
const unsigned int maxDimensions = 4;

TEST(MatrixUtilsTest, MatrixConstructorTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        for (unsigned int j = minDimensions; j <= maxDimensions; j++)
        {
            unsigned int numElements = i * j;
            Matrix<float> m(std::vector<float>(numElements, 1.0f), i, j);
            EXPECT_EQ(m.rows(), i);
            EXPECT_EQ(m.columns(), j);
            EXPECT_EQ(m.elements(), std::vector<float>(numElements, 1.0f));
        }
    }

    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        unsigned int numElements = i * i;
        Matrix<float> m(std::vector<float>(numElements, 1.0f), i);
        EXPECT_EQ(m.size(), i);
        EXPECT_EQ(m.columns(), m.columns());
        EXPECT_EQ(m.elements(), std::vector<float>(numElements, 1.0f));
    }
}

TEST(MatrixUtilsTest, MatrixCompMultTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        unsigned int numElements = i * i;
        Matrix<float> m1(std::vector<float>(numElements, 2.0f), i);
        Matrix<float> actualResult = m1.compMult(m1);
        std::vector<float> actualResultElements = actualResult.elements();
        std::vector<float> expectedResultElements(numElements, 4.0f);
        EXPECT_EQ(expectedResultElements, actualResultElements);
    }
}

TEST(MatrixUtilsTest, MatrixOuterProductTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        for (unsigned int j = minDimensions; j <= maxDimensions; j++)
        {
            unsigned int numElements = i * j;
            Matrix<float> m1(std::vector<float>(numElements, 2.0f), i, 1);
            Matrix<float> m2(std::vector<float>(numElements, 2.0f), 1, j);
            Matrix<float> actualResult = m1.outerProduct(m2);
            EXPECT_EQ(actualResult.rows(), i);
            EXPECT_EQ(actualResult.columns(), j);
            std::vector<float> actualResultElements = actualResult.elements();
            std::vector<float> expectedResultElements(numElements, 4.0f);
            EXPECT_EQ(expectedResultElements, actualResultElements);
        }
    }
}

TEST(MatrixUtilsTest, MatrixTransposeTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        for (unsigned int j = minDimensions; j <= maxDimensions; j++)
        {
            unsigned int numElements = i * j;
            Matrix<float> m1(std::vector<float>(numElements, 2.0f), i, j);
            Matrix<float> expectedResult = Matrix<float>(std::vector<float>(numElements, 2.0f), j, i);
            Matrix<float> actualResult = m1.transpose();
            EXPECT_EQ(expectedResult.elements(), actualResult.elements());
            EXPECT_EQ(actualResult.rows(), expectedResult.rows());
            EXPECT_EQ(actualResult.columns(), expectedResult.columns());
            // transpose(transpose(A)) = A
            Matrix<float> m2 = actualResult.transpose();
            EXPECT_EQ(m1.elements(), m2.elements());
        }
    }
}

TEST(MatrixUtilsTest, MatrixDeterminantTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        unsigned int numElements = i * i;
        Matrix<float> m(std::vector<float>(numElements, 2.0f), i);
        EXPECT_EQ(m.determinant(), 0.0f);
    }
}

TEST(MatrixUtilsTest, 2x2MatrixInverseTest)
{
    float inputElements[] =
    {
        2.0f, 5.0f,
        3.0f, 7.0f
    };
    unsigned int numElements = 4;
    std::vector<float> input(inputElements, inputElements + numElements);
    Matrix<float> inputMatrix(input, 2);
    float identityElements[] =
    {
        1.0f, 0.0f,
        0.0f, 1.0f
    };
    std::vector<float> identityMatrix(identityElements, identityElements + numElements);
    // A * inverse(A) = I, where I is identity matrix.
    Matrix<float> result = inputMatrix * inputMatrix.inverse();
    EXPECT_EQ(identityMatrix, result.elements());
}

TEST(MatrixUtilsTest, 3x3MatrixInverseTest)
{
    float inputElements[] =
    {
        11.0f, 23.0f, 37.0f,
        13.0f, 29.0f, 41.0f,
        19.0f, 31.0f, 43.0f
    };
    unsigned int numElements = 9;
    std::vector<float> input(inputElements, inputElements + numElements);
    Matrix<float> inputMatrix(input, 3);
    float identityElements[] =
    {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    std::vector<float> identityMatrix(identityElements, identityElements + numElements);
    // A * inverse(A) = I, where I is identity matrix.
    Matrix<float> result = inputMatrix * inputMatrix.inverse();
    std::vector<float> resultElements = result.elements();
    const float floatFaultTolarance = 0.000001f;
    for (size_t i = 0; i < numElements; i++)
        EXPECT_NEAR(resultElements[i], identityMatrix[i], floatFaultTolarance);
}

TEST(MatrixUtilsTest, 4x4MatrixInverseTest)
{
    float inputElements[] =
    {
        29.0f, 43.0f, 61.0f, 79.0f,
        31.0f, 47.0f, 67.0f, 83.0f,
        37.0f, 53.0f, 71.0f, 89.0f,
        41.0f, 59.0f, 73.0f, 97.0f
    };
    unsigned int numElements = 16;
    std::vector<float> input(inputElements, inputElements + numElements);
    Matrix<float> inputMatrix(input, 4);
    float identityElements[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    std::vector<float> identityMatrix(identityElements, identityElements + numElements);
    // A * inverse(A) = I, where I is identity matrix.
    Matrix<float> result = inputMatrix * inputMatrix.inverse();
    std::vector<float> resultElements = result.elements();
    const float floatFaultTolarance = 0.00001f;
    for (unsigned int i = 0; i < numElements; i++)
        EXPECT_NEAR(resultElements[i], identityMatrix[i], floatFaultTolarance);
}

static void checkMat4ExactlyEq(const Matrix<float>& a, const Matrix<float>& b) {
    for (unsigned int i = 0; i < 4; i++) {
        for (unsigned int j = 0; i < 4; i++) {
            EXPECT_EQ(a.at(i, j), b.at(i, j));
        }
    }
}

TEST(MatrixUtilsTest, Mat4Construction) {
    float elements[] =
    {
        0.0f, 1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f,
        12.0f, 13.0f, 14.0f, 15.0f,
    };

    std::vector<float> elementsVector(16, 0);
    for (int i = 0; i < 16; i++) {
        elementsVector[i] = elements[i];
    }

    Matrix<float> a(elements, 4);
    Mat4 b(elements);
    Mat4 bVec(elementsVector);

    checkMat4ExactlyEq(a, b);
    checkMat4ExactlyEq(b, bVec);

    a.setToIdentity();
    b = Mat4();

    checkMat4ExactlyEq(a, b);

    Mat4 c(0.0f, 1.0f, 2.0f, 3.0f,
           4.0f, 5.0f, 6.0f, 7.0f,
           8.0f, 9.0f, 10.0f, 11.0f,
           12.0f, 13.0f, 14.0f, 15.0f);
    Mat4 d(elements);
    Mat4 e(Matrix<float>(elements, 4));

    checkMat4ExactlyEq(c, d);
    checkMat4ExactlyEq(e, d);
}

struct RotateArgs {
    float angle;
    Vector3 axis;
};

// TODO: Spec out requirements for matrix precision
void checkMatrixCloseToGolden(float* golden, const Mat4& m) {
    const float floatFaultTolarance = 0.000001f;
    const auto& checkElts = m.elements();
    for (size_t i = 0; i < checkElts.size(); i++) {
        EXPECT_NEAR(golden[i], checkElts[i], floatFaultTolarance);
    }
}

void checkMatrixCloseToGolden(const std::vector<float>& golden, const Mat4& m) {
    const float floatFaultTolarance = 0.000001f;
    const auto& checkElts = m.elements();
    for (size_t i = 0; i < golden.size(); i++) {
        EXPECT_NEAR(golden[i], checkElts[i], floatFaultTolarance);
    }
}

TEST(MatrixUtilsTest, Mat4Rotate) {
    // Sanity check.
    float elementsExpected[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    std::vector<float> elementsExpectedVector(16, 0);
    for (int i = 0; i < 16; i++) {
        elementsExpectedVector[i] = elementsExpected[i];
    }

    Mat4 r = Mat4::rotate(0.f, Vector3(0.f, 0.f, 1.f));
    Mat4 golden(elementsExpected);
    checkMatrixCloseToGolden(elementsExpected, r);
    checkMatrixCloseToGolden(elementsExpectedVector, r);
    checkMat4ExactlyEq(r, golden);

    // Randomly-generated inputs, outputs using GLM.
    std::vector<RotateArgs> rotationGoldenInputs;
    std::vector<std::vector<float> > rotationGoldenOutputs;
    rotationGoldenInputs.push_back({ -123.202621,  Vector3(1.49511719, 8.43707085, 1.84892273)});
    rotationGoldenInputs.push_back({ -185.304932,  Vector3(8.33139801, 5.83176517, -5.52017164)});
    rotationGoldenInputs.push_back({ 89.1301575,  Vector3(-8.59627247, 2.45473671, -3.24616003)});
    rotationGoldenInputs.push_back({ 64.2547302,  Vector3(-1.96404457, -9.6942234, 9.19210434)});
    rotationGoldenInputs.push_back({ -298.558502,  Vector3(-5.36005449, -6.43335342, -5.37347507)});
    rotationGoldenInputs.push_back({ 288.26062,  Vector3(-2.30438757, -9.84476185, -0.912479401)});
    rotationGoldenInputs.push_back({ 142.39563,  Vector3(-1.00449753, -2.583498, -0.845117569)});
    rotationGoldenInputs.push_back({ -140.044586,  Vector3(-1.37100601, -2.50429916, -9.75720882)});
    rotationGoldenInputs.push_back({ -338.544342,  Vector3(6.80566216, 2.75088787, -5.83434296)});
    rotationGoldenInputs.push_back({ 79.0578613,  Vector3(9.05184937, -5.56159019, 6.35597992)});
    rotationGoldenInputs.push_back({ 4464.63672,  Vector3(-53.9424286, -10.361496, 54.3564453)});
    rotationGoldenInputs.push_back({ -2820.63477,  Vector3(62.1694794, 82.497757, -60.0084801)});
    rotationGoldenInputs.push_back({ 3371.05273,  Vector3(-74.5660324, -31.3026276, 96.725235)});
    rotationGoldenInputs.push_back({ 5501.7168,  Vector3(15.030838, 23.2323914, 66.8295593)});
    rotationGoldenInputs.push_back({ 392.175781,  Vector3(36.5722198, 69.2820892, 24.1789474)});
    rotationGoldenInputs.push_back({ -2206.71387,  Vector3(-91.5292282, 68.2716675, 42.0627289)});
    rotationGoldenInputs.push_back({ -4648.8623,  Vector3(50.7790375, 43.3964081, -36.3877525)});
    rotationGoldenInputs.push_back({ 2794.60156,  Vector3(76.2934265, 63.4901886, 79.5993042)});
    rotationGoldenInputs.push_back({ 2294.67871,  Vector3(-81.3662338, 77.694458, 10.8423462)});
    rotationGoldenInputs.push_back({ 2451.04688,  Vector3(-80.6299896, 51.8244629, 13.6877518)});
    rotationGoldenOutputs.push_back({-0.502578616, 0.077577576, 0.861043811, 0, 0.430558145, 0.886124551, 0.17147316, 0, -0.749689579, 0.456908166, -0.478748918, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.0388860106, 0.68008846, -0.732098162, 0, 0.768302619, -0.48879838, -0.413264215, 0, -0.638904691, -0.546402752, -0.541520953, -0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.819677353, -0.570997, 0.0457325876, 0, 0.111535847, 0.0807825029, -0.990471601, 0, 0.561861873, 0.816967964, 0.129902199, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.44633761, 0.672237933, 0.590659797, 0, -0.554105937, 0.725911558, -0.407454431, 0, -0.702673018, -0.145425856, 0.696492612, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.629537523, -0.292549372, 0.719789803, 0, 0.656178474, 0.696276903, -0.29090941, 0, -0.41606766, 0.655448973, 0.630297065, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.348719478, 0.236504108, -0.906896055, 0, 0.0657925531, 0.959072948, 0.27540952, 0, 0.934915006, -0.155707672, 0.318887234, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({-0.576898336, 0.375895619, 0.725183249, 0, 0.731807947, 0.632225156, 0.254457355, 0, -0.362829685, 0.67749095, -0.639813006, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({-0.734417081, 0.6750319, 0.0704519302, 0, -0.557663441, -0.659350932, 0.504249811, 0, 0.386837244, 0.331041187, 0.860678971, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.967206657, -0.212837532, -0.138605639, 0, 0.242349193, 0.936665297, 0.252833962, 0, 0.0760145336, -0.278133631, 0.957529902, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.622941673, 0.237952977, 0.745199323, 0, -0.770188689, 0.353324354, 0.531009674, 0, -0.136941731, -0.904732108, 0.403369129, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.069116354, 0.57701838, -0.813801348, 0, -0.23717764, -0.782857955, -0.575221837, -0, -0.969004452, 0.232772723, 0.0827475786, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.642379999, -0.255957425, -0.722380519, 0, 0.608449876, 0.743438125, 0.277648032, 0, 0.465979189, -0.617887914, 0.63330704, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({-0.0772442222, 0.821814537, -0.56449455, 0, -0.335262537, -0.55462563, -0.761570334, -0, -0.938952804, 0.130427033, 0.318365276, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({-0.15112105, 0.98498112, -0.0835132897, 0, -0.824382067, -0.0789582208, 0.560499489, 0, 0.545487404, 0.153550103, 0.823933125, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.876976967, 0.214932591, -0.429785222, 0, -0.0991527736, 0.95608449, 0.275809884, 0, 0.470191568, -0.199264526, 0.859775245, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.863490582, -0.384278059, 0.32667169, 0, 0.118962884, 0.78459096, 0.608493924, 0, -0.490134507, -0.486566991, 0.723201752, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.920134842, -0.192495584, -0.341023833, 0, 0.302240282, 0.902822196, 0.305880517, 0, 0.249003246, -0.384522468, 0.888898134, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.41095221, -0.348385721, 0.842464209, 0, 0.898837209, 0.309263796, -0.310560435, 0, -0.152348787, 0.884863555, 0.440234661, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.179531276, -0.77461791, -0.606411994, 0, -0.911041379, 0.101665795, -0.399584115, 0, 0.371176422, 0.624204338, -0.687456906, 0, 0, 0, 0, 1, });
    rotationGoldenOutputs.push_back({0.803580999, -0.41760689, 0.424101382, 0, -0.153726518, 0.542743206, 0.825710595, 0, -0.575000584, -0.728720963, 0.371940881, 0, 0, 0, 0, 1, });

    EXPECT_EQ(rotationGoldenInputs.size(), rotationGoldenOutputs.size());

    for (size_t i = 0; i < rotationGoldenInputs.size(); i++) {
        const auto& input = rotationGoldenInputs[i];
        const auto& output = rotationGoldenOutputs[i];
        Mat4 r = Mat4::rotate(input.angle, input.axis);
        checkMatrixCloseToGolden(output, r);
    }

}

struct TranslateArgs {
    float x;
    float y;
    float z;
};

TEST(MatrixUtilsTest, Mat4Translate) {

    // Sanity check.
    float elementsExpected[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    std::vector<float> elementsExpectedVector(16, 0);
    for (int i = 0; i < 16; i++) {
        elementsExpectedVector[i] = elementsExpected[i];
    }

    Mat4 r = Mat4::translate(Vector3(0.f, 0.f, 0.f));
    Mat4 golden(elementsExpected);
    checkMatrixCloseToGolden(elementsExpected, r);
    checkMatrixCloseToGolden(elementsExpectedVector, r);
    checkMat4ExactlyEq(r, golden);

    // Randomly-generated inputs, outputs using GLM.
    std::vector<TranslateArgs> translationGoldenInputs;
    std::vector<std::vector<float> > translationGoldenOutputs;

    translationGoldenInputs.push_back({ 5.01395607, 9.55217743, 3.45331192});
    translationGoldenInputs.push_back({ -0.186609268, 3.66242599, -5.59755707});
    translationGoldenInputs.push_back({ 1.86068058, -7.55047512, -0.839432716});
    translationGoldenInputs.push_back({ -8.34768963, 0.377576828, 5.92378044});
    translationGoldenInputs.push_back({ 0.97728157, 5.16618156, 8.00214958});
    translationGoldenInputs.push_back({ -7.86507416, -8.29834652, 9.68736839});
    translationGoldenInputs.push_back({ -4.40079975, -4.24009275, -3.23374319});
    translationGoldenInputs.push_back({ -9.52563572, 2.63376999, 5.77368164});
    translationGoldenInputs.push_back({ -1.73216724, 7.47092438, 3.82664871});
    translationGoldenInputs.push_back({ -5.50556374, 7.99264145, -7.67483854});
    translationGoldenInputs.push_back({ 89.9044342, 23.7133865, -49.0459747});
    translationGoldenInputs.push_back({ 84.322937, 15.5694809, 76.3803406});
    translationGoldenInputs.push_back({ -75.6585083, 7.44341278, -98.5154114});
    translationGoldenInputs.push_back({ 51.4982758, -68.4115524, 7.06030273});
    translationGoldenInputs.push_back({ 62.3893738, -21.8546677, 88.6265564});
    translationGoldenInputs.push_back({ -53.6188774, 27.5188293, -91.0673065});
    translationGoldenInputs.push_back({ 31.7216339, -54.6880989, 57.1137543});
    translationGoldenInputs.push_back({ -89.1680908, -48.1198616, 49.4577789});
    translationGoldenInputs.push_back({ 36.7780151, -71.8430481, -66.198288});
    translationGoldenInputs.push_back({ 5.47250366, -23.6154709, 94.720108});
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 5.01395607, 9.55217743, 3.45331192, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -0.186609268, 3.66242599, -5.59755707, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1.86068058, -7.55047512, -0.839432716, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -8.34768963, 0.377576828, 5.92378044, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0.97728157, 5.16618156, 8.00214958, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -7.86507416, -8.29834652, 9.68736839, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -4.40079975, -4.24009275, -3.23374319, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -9.52563572, 2.63376999, 5.77368164, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -1.73216724, 7.47092438, 3.82664871, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -5.50556374, 7.99264145, -7.67483854, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 89.9044342, 23.7133865, -49.0459747, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 84.322937, 15.5694809, 76.3803406, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -75.6585083, 7.44341278, -98.5154114, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 51.4982758, -68.4115524, 7.06030273, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 62.3893738, -21.8546677, 88.6265564, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -53.6188774, 27.5188293, -91.0673065, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 31.7216339, -54.6880989, 57.1137543, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -89.1680908, -48.1198616, 49.4577789, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 36.7780151, -71.8430481, -66.198288, 1, });
    translationGoldenOutputs.push_back({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 5.47250366, -23.6154709, 94.720108, 1, });

    EXPECT_EQ(translationGoldenInputs.size(), translationGoldenOutputs.size());

    for (size_t i = 0; i < translationGoldenInputs.size(); i++) {
        const auto& input = translationGoldenInputs[i];
        const auto& output = translationGoldenOutputs[i];
        Mat4 r = Mat4::translate(Vector3(input.x, input.y, input.z));
        checkMatrixCloseToGolden(output, r);
    }
}

struct ScaleArgs {
    float x;
    float y;
    float z;
};

TEST(MatrixUtilsTest, Mat4Scale) {

    // Sanity check.
    float elementsExpected[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    std::vector<float> elementsExpectedVector(16, 0);
    for (int i = 0; i < 16; i++) {
        elementsExpectedVector[i] = elementsExpected[i];
    }

    Mat4 r = Mat4::scale(Vector3(1.f, 1.f, 1.f));
    Mat4 golden(elementsExpected);
    checkMatrixCloseToGolden(elementsExpected, r);
    checkMatrixCloseToGolden(elementsExpectedVector, r);
    checkMat4ExactlyEq(r, golden);

    // Randomly-generated inputs, outputs using GLM.
    std::vector<ScaleArgs> scaleGoldenInputs;
    std::vector<std::vector<float> > scaleGoldenOutputs;

    scaleGoldenInputs.push_back({ -3.8996706, -1.76555729, 6.28096008 });
    scaleGoldenInputs.push_back({ 4.09708786, -0.245664597, -8.88993263 });
    scaleGoldenInputs.push_back({ 6.91117859, -3.84439659, 7.22478676 });
    scaleGoldenInputs.push_back({ 6.99506569, 6.06585121, 8.75965309 });
    scaleGoldenInputs.push_back({ 3.50158215, -8.913311, -6.01795244 });
    scaleGoldenInputs.push_back({ -3.72835541, -2.46027231, -9.79825687 });
    scaleGoldenInputs.push_back({ 0.69778347, 7.64270401, -9.09115982 });
    scaleGoldenInputs.push_back({ 4.88454056, -5.51010418, -8.32338142 });
    scaleGoldenInputs.push_back({ 8.93768692, -4.29102612, 0.723935127 });
    scaleGoldenInputs.push_back({ 7.17598343, 6.75212288, 2.91799545 });
    scaleGoldenInputs.push_back({ 27.4279022, -19.206604, -5.40982819 });
    scaleGoldenInputs.push_back({ 77.0056458, 33.8750153, -62.6624527 });
    scaleGoldenInputs.push_back({ 32.1618042, -56.4901123, -29.2870483 });
    scaleGoldenInputs.push_back({ -27.4239578, 85.466217, 30.6318054 });
    scaleGoldenInputs.push_back({ 28.7731171, -10.1281204, -23.3521729 });
    scaleGoldenInputs.push_back({ -79.8985138, -54.3800964, 33.753006 });
    scaleGoldenInputs.push_back({ 86.6550598, 11.5293961, -25.3433609 });
    scaleGoldenInputs.push_back({ 54.1160889, -71.0586243, -82.2456818 });
    scaleGoldenInputs.push_back({ 96.7931213, 2.15691376, 51.1624756 });
    scaleGoldenInputs.push_back({ 87.7290497, 62.0530243, -74.9053574 });
    scaleGoldenOutputs.push_back({-3.8996706, -0, -0, -0, -0, -1.76555729, -0, -0, 0, 0, 6.28096008, 0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({4.09708786, 0, 0, 0, -0, -0.245664597, -0, -0, -0, -0, -8.88993263, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({6.91117859, 0, 0, 0, -0, -3.84439659, -0, -0, 0, 0, 7.22478676, 0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({6.99506569, 0, 0, 0, 0, 6.06585121, 0, 0, 0, 0, 8.75965309, 0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({3.50158215, 0, 0, 0, -0, -8.913311, -0, -0, -0, -0, -6.01795244, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({-3.72835541, -0, -0, -0, -0, -2.46027231, -0, -0, -0, -0, -9.79825687, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({0.69778347, 0, 0, 0, 0, 7.64270401, 0, 0, -0, -0, -9.09115982, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({4.88454056, 0, 0, 0, -0, -5.51010418, -0, -0, -0, -0, -8.32338142, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({8.93768692, 0, 0, 0, -0, -4.29102612, -0, -0, 0, 0, 0.723935127, 0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({7.17598343, 0, 0, 0, 0, 6.75212288, 0, 0, 0, 0, 2.91799545, 0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({27.4279022, 0, 0, 0, -0, -19.206604, -0, -0, -0, -0, -5.40982819, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({77.0056458, 0, 0, 0, 0, 33.8750153, 0, 0, -0, -0, -62.6624527, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({32.1618042, 0, 0, 0, -0, -56.4901123, -0, -0, -0, -0, -29.2870483, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({-27.4239578, -0, -0, -0, 0, 85.466217, 0, 0, 0, 0, 30.6318054, 0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({28.7731171, 0, 0, 0, -0, -10.1281204, -0, -0, -0, -0, -23.3521729, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({-79.8985138, -0, -0, -0, -0, -54.3800964, -0, -0, 0, 0, 33.753006, 0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({86.6550598, 0, 0, 0, 0, 11.5293961, 0, 0, -0, -0, -25.3433609, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({54.1160889, 0, 0, 0, -0, -71.0586243, -0, -0, -0, -0, -82.2456818, -0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({96.7931213, 0, 0, 0, 0, 2.15691376, 0, 0, 0, 0, 51.1624756, 0, 0, 0, 0, 1, });
    scaleGoldenOutputs.push_back({87.7290497, 0, 0, 0, 0, 62.0530243, 0, 0, -0, -0, -74.9053574, -0, 0, 0, 0, 1, });

    EXPECT_EQ(scaleGoldenInputs.size(), scaleGoldenOutputs.size());

    for (size_t i = 0; i < scaleGoldenInputs.size(); i++) {
        const auto& input = scaleGoldenInputs[i];
        const auto& output = scaleGoldenOutputs[i];
        Mat4 r = Mat4::scale(Vector3(input.x, input.y, input.z));
        checkMatrixCloseToGolden(output, r);
    }
}

struct FrustumArgs {
    float l; float r;
    float b; float t;
    float n; float f;
};

TEST(MatrixUtilsTest, Mat4Frustum) {
    // Randomly-generated inputs, outputs using GLM.
    std::vector<FrustumArgs> frustumGoldenInputs;
    std::vector<std::vector<float> > frustumGoldenOutputs;

    EXPECT_EQ(frustumGoldenInputs.size(), frustumGoldenOutputs.size());

    frustumGoldenInputs.push_back({ -3.8996706, -1.76555729, 6.28096008, 4.09708786, -0.245664597, -8.88993263 });
    frustumGoldenInputs.push_back({ 6.91117859, -3.84439659, 7.22478676, 6.99506569, 6.06585121, 8.75965309 });
    frustumGoldenInputs.push_back({ 3.50158215, -8.913311, -6.01795244, -3.72835541, -2.46027231, -9.79825687 });
    frustumGoldenInputs.push_back({ 0.69778347, 7.64270401, -9.09115982, 4.88454056, -5.51010418, -8.32338142 });
    frustumGoldenInputs.push_back({ 8.93768692, -4.29102612, 0.723935127, 7.17598343, 6.75212288, 2.91799545 });
    frustumGoldenInputs.push_back({ 2.74279022, -1.92066097, -0.5409832, 7.70056534, 3.38750076, -6.26624537 });
    frustumGoldenInputs.push_back({ 3.2161808, -5.64901114, -2.92870474, -2.74239588, 8.54662132, 3.06318092 });
    frustumGoldenInputs.push_back({ 2.87731171, -1.01281261, -2.335217, -7.98985195, -5.43800926, 3.37530041 });
    frustumGoldenInputs.push_back({ 8.66550636, 1.1529398, -2.53433657, 5.41160774, -7.10586214, -8.22456837 });
    frustumGoldenInputs.push_back({ 9.67931366, 0.215691566, 5.11624813, 8.77290535, 6.20530319, -7.49053574 });
    frustumGoldenInputs.push_back({ 65.6408844, 26.1959991, 76.2994385, -35.3414688, 15.9715195, 33.2543182 });
    frustumGoldenInputs.push_back({ -94.5709152, -53.3344078, 8.66033173, -45.8301888, -67.9809113, 44.7441406 });
    frustumGoldenInputs.push_back({ 14.8764267, 28.0944366, -16.8030472, -8.82382202, 97.9778595, -86.2041702 });
    frustumGoldenInputs.push_back({ -33.4564056, 98.2350159, 35.8691254, 52.35672, -40.6093903, 77.9731903 });
    frustumGoldenInputs.push_back({ 95.3814087, 75.2443695, 32.0545502, -59.051651, -81.1143951, -89.6266022 });
    frustumGoldenInputs.push_back({ 45.7317352, 13.2782669, -32.0627747, -79.0900269, -66.076561, 51.2055969 });
    frustumGoldenInputs.push_back({ 12.5197067, 18.7676086, 27.2567368, -96.0218506, -39.2679329, 23.7942581 });
    frustumGoldenInputs.push_back({ -89.8873825, 62.763031, 58.2431641, 92.890686, 13.859436, -64.4221039 });
    frustumGoldenInputs.push_back({ 57.7148743, 13.6885071, 62.6116638, -85.8590393, -32.8254318, -97.0158539 });
    frustumGoldenInputs.push_back({ 54.4938049, 77.3837738, -11.0927963, -36.6919518, -81.641983, 43.1220703 });
    frustumGoldenOutputs.push_back({ -0.230226383, 0, 0, 0, 0, 0.224980742, 0, 0, -2.65460491, -4.75213146, -1.05683875, -1, 0, 0, 0.505292416, 0, });
    frustumGoldenOutputs.push_back({ -1.12794542, 0, 0, 0, 0, -52.8105774, 0, 0, -0.285134166, -61.9005165, -5.5035615, -1, 0, 0, -39.4496384, 0, });
    frustumGoldenOutputs.push_back({ 0.396342099, 0, 0, 0, 0, -2.14908767, 0, 0, 0.435906202, -4.25677824, -1.67055809, -1, 0, 0, 6.5703001, 0, });
    frustumGoldenOutputs.push_back({ -1.58680117, 0, 0, 0, 0, -0.788526356, 0, 0, 1.20094788, -0.300995231, -4.91721392, -1, 0, 0, 32.6044617, 0, });
    frustumGoldenOutputs.push_back({ -1.02082837, 0, 0, 0, 0, 2.09301686, 0, 0, -0.351255685, 1.22440469, 2.52211714, -1, 0, 0, 10.2775211, 0, });
    frustumGoldenOutputs.push_back({ -1.45278704, 0, 0, 0, 0, 0.822054446, 0, 0, -0.176292017, 0.868718088, -0.298199743, -1, 0, 0, -4.3976531, 0, });
    frustumGoldenOutputs.push_back({ -1.92812991, 0, 0, 0, 0, 91.7468033, 0, 0, 0.274425, -30.4392433, 2.11724782, -1, 0, 0, 9.54869461, 0, });
    frustumGoldenOutputs.push_back({ 2.79580235, 0, 0, 0, 0, 1.92338121, 0, 0, -0.479290366, 1.825948, 0.234044746, -1, 0, 0, 4.16527176, 0, });
    frustumGoldenOutputs.push_back({ 1.89172685, 0, 0, 0, 0, -1.78855073, 0, 0, -1.30693638, 0.362105638, -13.7037144, -1, 0, 0, 104.482567, 0, });
    frustumGoldenOutputs.push_back({ -1.31140125, 0, 0, 0, 0, 3.39397597, 0, 0, -1.04558325, 3.79831982, -0.0938410982, -1, 0, 0, -6.78761578, 0, });
    frustumGoldenOutputs.push_back({ -0.809814453, 0, 0, 0, 0, -0.286123067, 0, 0, -2.328233, -0.36687243, -2.84825611, -1, 0, 0, -61.4625015, 0, });
    frustumGoldenOutputs.push_back({ -3.29712272, 0, 0, 0, 0, 2.49514627, 0, 0, -3.58675671, 0.68213439, 0.206136703, -1, 0, 0, 53.9675522, 0, });
    frustumGoldenOutputs.push_back({ 14.8249035, 0, 0, 0, 0, 24.558239, 0, 0, 3.25093293, -3.21169901, 0.0639242008, -1, 0, 0, -91.7146912, 0, });
    frustumGoldenOutputs.push_back({ -0.616735518, 0, 0, 0, 0, -4.926054, 0, 0, 0.491896957, 5.35104418, -0.315086752, -1, 0, 0, 53.4048691, 0, });
    frustumGoldenOutputs.push_back({ 8.05623817, 0, 0, 0, 0, 1.78065586, 0, 0, -8.47323036, 0.296325624, -20.0583706, -1, 0, 0, 1708.13696, 0, });
    frustumGoldenOutputs.push_back({ 4.07208014, 0, 0, 0, 0, 2.81013918, 0, 0, -1.81829572, 2.36358285, 0.126796469, -1, 0, 0, 57.6982841, 0, });
    frustumGoldenOutputs.push_back({ -12.5699577, 0, 0, 0, 0, 0.637060046, 0, 0, 5.00765133, 0.557802558, 0.24537167, -1, 0, 0, 29.6326942, 0, });
    frustumGoldenOutputs.push_back({ 0.181583986, 0, 0, 0, 0, 0.800024688, 0, 0, -0.177689329, 4.36203909, -0.645907938, -1, 0, 0, -22.8113556, 0, });
    frustumGoldenOutputs.push_back({ 1.49117148, 0, 0, 0, 0, 0.442180604, 0, 0, -1.62183225, 0.156578869, -2.02275157, -1, 0, 0, 99.2231369, 0, });
    frustumGoldenOutputs.push_back({ -7.13342905, 0, 0, 0, 0, 6.37849045, 0, 0, 5.76137018, 1.86665332, 0.308742076, -1, 0, 0, 56.4356689, 0, });

    for (size_t i = 0; i < frustumGoldenInputs.size(); i++) {
        const auto& input = frustumGoldenInputs[i];
        const auto& output = frustumGoldenOutputs[i];
        Mat4 r = Mat4::frustum(input.l, input.r, input.b, input.t, input.n, input.f);
        checkMatrixCloseToGolden(output, r);
    }
}

TEST(MatrixUtilsTest, Mat4Ortho) {
    // Randomly-generated inputs, outputs using GLM.
    std::vector<FrustumArgs> orthoGoldenInputs;
    std::vector<std::vector<float> > orthoGoldenOutputs;

    orthoGoldenInputs.push_back({ -1.25152302, 5.65236187, -0.742723465, -2.96575642, -5.4732933, -9.64169025 });
    orthoGoldenInputs.push_back({ -7.8948307, -8.41461182, -4.38939524, 7.43927383, -8.12619114, 3.11079788 });
    orthoGoldenInputs.push_back({ 3.16677094, 3.91341686, -7.19939613, -0.251950264, 5.46253586, 8.83205605 });
    orthoGoldenInputs.push_back({ 0.359718323, 5.7859621, 4.67861748, -6.47365665, -2.75105572, 2.99775314 });
    orthoGoldenInputs.push_back({ 3.2480135, 9.35517502, -7.59222412, -2.51350832, -4.52820015, -5.45645809 });
    orthoGoldenInputs.push_back({ -6.69183254, -9.6249752, -6.95916653, -2.70991373, -5.51966906, -9.07919693 });
    orthoGoldenInputs.push_back({ 5.93864536, -9.17840195, -1.40781021, -1.05528927, 3.75637054, -6.67154312 });
    orthoGoldenInputs.push_back({ -8.623806, -0.299538612, 5.66238213, 7.64834213, 5.66864109, -7.14568996 });
    orthoGoldenInputs.push_back({ 2.38575554, -2.60208368, 6.78413582, 0.986829758, 5.64635181, -1.75970745 });
    orthoGoldenInputs.push_back({ 4.60285378, 0.175751686, -6.14346075, 6.85240936, 8.43806267, -1.48247528 });
    orthoGoldenInputs.push_back({ 40.3382721, -34.5338974, -11.1726379, 21.4206924, 17.5087891, 70.273407 });
    orthoGoldenInputs.push_back({ 85.2872314, 22.3899002, -92.853714, 7.60593414, 32.9500732, -8.13745117 });
    orthoGoldenInputs.push_back({ 33.8771057, -27.6973648, 90.3841095, 85.8473358, 36.0423279, -36.5140991 });
    orthoGoldenInputs.push_back({ -92.4729462, 7.15921021, -75.2177963, 14.4945984, 10.6297379, 53.9828796 });
    orthoGoldenInputs.push_back({ 90.2037659, 54.5332947, -58.9839516, 56.7301331, 63.240387, 81.1043854 });
    orthoGoldenInputs.push_back({ -78.6419067, 65.515625, -78.8265305, -37.5036049, 76.9322205, -0.221328735 });
    orthoGoldenInputs.push_back({ 80.1368561, 60.2946777, -27.3523788, 88.5419617, -75.3445969, 83.3852081 });
    orthoGoldenInputs.push_back({ 55.0712585, -17.4033813, -98.6088104, 81.6600952, 61.1217804, 73.5973816 });
    orthoGoldenInputs.push_back({ -48.7522583, 20.8100433, -45.6365356, -13.2819519, -29.7577133, 62.1014862 });
    orthoGoldenInputs.push_back({ -60.3634567, 71.4023285, 59.0719757, 22.6195831, -32.6802139, -56.3766899 });
    orthoGoldenOutputs.push_back({0.289691955, 0, 0, 0, 0, -0.899671793, 0, 0, 0, 0, 0.479800761, 0, -0.637443841, -1.66820729, -3.62609029, 1, });
    orthoGoldenOutputs.push_back({-3.84777355, 0, 0, 0, 0, 0.169080734, 0, 0, 0, 0, -0.177983627, 0, -31.3775196, -0.257837862, 0.446328938, 1, });
    orthoGoldenOutputs.push_back({2.67864585, 0, 0, 0, 0, 0.287875593, 0, 0, 0, 0, -0.593556345, 0, -9.48265743, 1.07253027, -4.24232244, 1, });
    orthoGoldenOutputs.push_back({0.36857909, 0, 0, 0, 0, -0.179335624, 0, 0, 0, 0, -0.347898155, 0, -1.13258469, -0.160957232, -0.0429127887, 1, });
    orthoGoldenOutputs.push_back({0.327484369, 0, 0, 0, 0, 0.393800348, 0, 0, 0, 0, 2.15457344, 0, -2.06367373, 1.98982036, -10.75634, 1, });
    orthoGoldenOutputs.push_back({-0.681862533, 0, 0, 0, 0, 0.470670998, 0, 0, 0, 0, 0.561872244, 0, -5.5629096, 2.27547789, -4.10134888, 1, });
    orthoGoldenOutputs.push_back({-0.132300973, 0, 0, 0, 0, 5.67342186, 0, 0, 0, 0, 0.19179292, 0, -0.214311466, 6.98710108, -0.279554725, 1, });
    orthoGoldenOutputs.push_back({0.240261391, 0, 0, 0, 0, 1.00706959, 0, 0, 0, 0, 0.156075254, 0, 1.0719676, -6.70241308, -0.115265392, 1, });
    orthoGoldenOutputs.push_back({-0.400975227, 0, 0, 0, 0, -0.34498781, 0, 0, 0, 0, 0.270049155, 0, -0.0433711149, 1.34044421, 0.524792492, 1, });
    orthoGoldenOutputs.push_back({-0.451762795, 0, 0, 0, 0, 0.153895035, 0, 0, 0, 0, 0.201601967, 0, 1.07939804, -0.0545518361, 0.701130033, 1, });
    orthoGoldenOutputs.push_back({-0.0267121959, 0, 0, 0, 0, 0.0613622479, 0, 0, 0, 0, -0.037904188, 0, 0.0775237978, -0.314421833, -1.66365647, 1, });
    orthoGoldenOutputs.push_back({-0.0317978524, 0, 0, 0, 0, 0.0199084915, 0, 0, 0, 0, 0.0486765765, 0, 1.71195078, 0.848577321, 0.603896737, 1, });
    orthoGoldenOutputs.push_back({-0.0324809924, 0, 0, 0, 0, -0.440841913, 0, 0, 0, 0, 0.0275647528, 0, 0.100362062, 38.8451042, -0.00650212867, 1, });
    orthoGoldenOutputs.push_back({0.0200738404, 0, 0, 0, 0, 0.0222934633, 0, 0, 0, 0, -0.0461327583, 0, 0.856287181, 0.67686522, -1.4903791, 1, });
    orthoGoldenOutputs.push_back({-0.0560687855, 0, 0, 0, 0, 0.0172839817, 0, 0, 0, 0, -0.111957021, 0, 4.05761576, 0.0194774792, -8.08020496, 1, });
    orthoGoldenOutputs.push_back({0.0138737112, 0, 0, 0, 0, 0.0483992845, 0, 0, 0, 0, 0.0259223338, 0, 0.091055125, 2.81514764, 0.994262636, 1, });
    orthoGoldenOutputs.push_back({-0.100795388, 0, 0, 0, 0, 0.0172570981, 0, 0, 0, 0, -0.0126000289, 0, 7.077425, -0.527977288, -0.0506559648, 1, });
    orthoGoldenOutputs.push_back({-0.027595859, 0, 0, 0, 0, 0.0110945366, 0, 0, 0, 0, -0.160312921, 0, 0.519738734, 0.0940190703, -10.7986107, 1, });
    orthoGoldenOutputs.push_back({0.0287512057, 0, 0, 0, 0, 0.061815042, 0, 0, 0, 0, -0.0217724517, 0, 0.401686162, 1.82102442, -0.352101624, 1, });
    orthoGoldenOutputs.push_back({0.0151784476, 0, 0, 0, 0, -0.054866083, 0, 0, 0, 0, 0.0844007358, 0, -0.083776474, 2.24104786, -3.75823402, 1, });

    EXPECT_EQ(orthoGoldenInputs.size(), orthoGoldenOutputs.size());

    for (size_t i = 0; i < orthoGoldenInputs.size(); i++) {
        const auto& input = orthoGoldenInputs[i];
        const auto& output = orthoGoldenOutputs[i];
        Mat4 r = Mat4::ortho(input.l, input.r, input.b, input.t, input.n, input.f);
        checkMatrixCloseToGolden(output, r);
    }
}

TEST(MatrixUtilsTest, Mat4InvTr) {

    std::vector<std::vector<float> > invTrInputs;
    std::vector<std::vector<float> > invTrOutputs;
    invTrInputs.push_back({0.324787796, -0.82795614, 0.457166672, 0, 0.904120147, 0.129906356, -0.407051563, 0, 0.277631968, 0.545539081, 0.790763855, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({-0.884383917, 0.304780543, -0.353516459, 0, 0.46478039, 0.6447106, -0.606899917, 0, 0.0429445058, -0.701040089, -0.711827636, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({-0.448778689, 0.863298535, 0.230896652, 0, -0.576556444, -0.0823006034, -0.812901735, -0, -0.682773948, -0.497937948, 0.534675121, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.986561418, 0.0901400745, -0.136276588, 0, 0.0155128352, 0.778610766, 0.627315342, 0, 0.162652686, -0.620999157, 0.766749024, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({-0.556666374, 0.830273747, -0.0277124047, 0, -0.0281167924, 0.0145094991, 0.999499321, 0, 0.830260158, 0.557166934, 0.0152677298, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.416067332, -0.909328103, 0.00322183967, 0, -0.139758497, -0.0674473643, -0.987885833, -0, 0.898529649, 0.410576731, -0.155148983, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.903989196, -0.396780759, -0.159275174, 0, 0.413967699, 0.905429363, 0.0939593166, 0, 0.10693118, -0.15087299, 0.9827528, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.36621353, 0.218485758, 0.904517412, 0, -0.665167749, 0.741217852, 0.0902667642, 0, -0.650722384, -0.634712696, 0.416773677, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.379142672, 0.727737248, -0.571532369, 0, -0.351137936, 0.6845994, 0.638768911, 0, 0.856126666, -0.0414978564, 0.515097022, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({-0.67668426, -0.095326364, -0.730076253, -0, -0.592209458, 0.659645498, 0.462769926, 0, 0.437477291, 0.74550724, -0.502824783, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.801266789, -0.542922556, 0.251409054, 0, 0.33842048, 0.757788777, 0.557877839, 0, -0.493399441, -0.361927032, 0.790927112, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.610097289, 0.405100673, 0.680936694, 0, -0.647082686, 0.750701249, 0.133160397, 0, -0.457236648, -0.521863163, 0.720134377, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.630416036, 0.27964893, -0.724135339, 0, 0.373588979, 0.708414316, 0.598815858, 0, 0.680446088, -0.648032129, 0.342121989, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({-0.288822055, 0.927869499, -0.235881656, 0, 0.889969707, 0.169390857, -0.423391968, 0, -0.352896273, -0.332212448, -0.874699414, -0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.787862301, 0.286684394, 0.545055151, 0, -0.52067095, 0.782737195, 0.340916812, 0, -0.328899384, -0.55238986, 0.765957355, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({-0.322537899, 0.9464975, 0.0105716586, 0, -0.492300808, -0.158201277, -0.855927706, -0, -0.80846101, -0.281273603, 0.516987443, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.313779384, 0.365670979, 0.876257539, 0, 0.876765549, 0.242643148, -0.415218651, 0, -0.364451259, 0.898559511, -0.244471267, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.886239588, -0.383178651, 0.260295004, 0, 0.441694558, 0.868344069, -0.225575864, 0, -0.139589757, 0.314885139, 0.938808799, 0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.461484134, -0.863555372, -0.203235611, 0, -0.749236643, -0.502052307, 0.431958348, 0, -0.47505486, -0.0470703244, -0.878696322, -0, 0, 0, 0, 1, });
    invTrInputs.push_back({0.732764006, -0.507641852, 0.453162998, 0, -0.070279792, 0.605925083, 0.792411208, 0, -0.676843941, -0.612498641, 0.408323109, 0, 0, 0, 0, 1, });
    invTrOutputs.push_back({0.324787885, -0.827956259, 0.457166851, -0, 0.904120326, 0.129906401, -0.407051593, 0, 0.277632058, 0.545539081, 0.790763974, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({-0.884384036, 0.304780513, -0.353516459, -0, 0.46478042, 0.644710541, -0.606899917, -0, 0.0429445207, -0.701040089, -0.711827755, 0, -0, 0, 0, 1, });
    invTrOutputs.push_back({-0.448778749, 0.863298595, 0.230896622, -0, -0.576556504, -0.0823005959, -0.812901735, 0, -0.682774007, -0.497937948, 0.534675121, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.986561418, 0.0901400819, -0.136276603, -0, 0.0155128324, 0.778610826, 0.627315402, -0, 0.162652686, -0.620999217, 0.766749084, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({-0.556666493, 0.830273807, -0.0277124085, -0, -0.0281168316, 0.0145094739, 0.999499381, -0, 0.830260217, 0.557166934, 0.0152676851, -0, -0, -0, -0, 1, });
    invTrOutputs.push_back({0.416067332, -0.909328103, 0.00322187319, -0, -0.139758527, -0.067447342, -0.987885833, 0, 0.898529649, 0.410576731, -0.155148968, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.903989196, -0.396780759, -0.159275174, -0, 0.413967729, 0.905429423, 0.0939593166, 0, 0.10693118, -0.150872976, 0.9827528, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.366213471, 0.218485773, 0.904517233, -0, -0.66516757, 0.741217673, 0.0902667865, 0, -0.650722265, -0.634712517, 0.416773587, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.379142731, 0.727737367, -0.571532428, -0, -0.351137996, 0.684599519, 0.638769031, -0, 0.856126845, -0.0414978676, 0.515097141, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({-0.67668426, -0.0953262374, -0.730076134, -0, -0.592209458, 0.659645259, 0.462769806, 0, 0.437477261, 0.745507002, -0.502824783, -0, -0, -0, -0, 1, });
    invTrOutputs.push_back({0.801266909, -0.542922616, 0.251409084, -0, 0.33842048, 0.757788837, 0.557877898, 0, -0.493399531, -0.361927092, 0.790927172, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.610097229, 0.405100614, 0.680936694, -0, -0.647082627, 0.750701129, 0.133160412, 0, -0.457236618, -0.521863103, 0.720134377, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.630416095, 0.27964893, -0.724135458, -0, 0.373588949, 0.708414316, 0.598815918, -0, 0.680446148, -0.648032129, 0.342122048, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({-0.288822144, 0.92786932, -0.235881567, -0, 0.889969587, 0.169390693, -0.423391908, -0, -0.352896243, -0.332212448, -0.874699414, 0, -0, 0, 0, 1, });
    invTrOutputs.push_back({0.787862301, 0.286684424, 0.545055151, -0, -0.52067095, 0.782737195, 0.340916842, 0, -0.328899413, -0.55238986, 0.765957355, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({-0.322537929, 0.9464975, 0.0105716586, -0, -0.492300838, -0.158201277, -0.855927706, 0, -0.80846101, -0.281273574, 0.516987383, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.313779354, 0.36567089, 0.876257479, -0, 0.87676549, 0.242643088, -0.415218651, 0, -0.364451259, 0.898559332, -0.244471282, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.886239588, -0.383178681, 0.260295033, -0, 0.441694558, 0.868344069, -0.225575879, 0, -0.139589787, 0.314885169, 0.938808858, -0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.461483896, -0.863555253, -0.203235537, -0, -0.749236524, -0.502052426, 0.431958318, 0, -0.475054801, -0.0470703468, -0.878696382, 0, -0, 0, -0, 1, });
    invTrOutputs.push_back({0.732763946, -0.507641852, 0.453162938, -0, -0.0702798143, 0.605925024, 0.792411149, 0, -0.676843822, -0.612498522, 0.408323079, -0, -0, 0, -0, 1, });

    EXPECT_EQ(invTrInputs.size(), invTrInputs.size());

    for (size_t i = 0; i < invTrInputs.size(); i++) {
        Mat4 a(invTrInputs[i]);
        checkMatrixCloseToGolden(invTrOutputs[i], a.transpose().inverse());
    }
}

TEST(MatrixUtilsTest, Mat4Mult) {
    Mat4 a, b;
    checkMatrixCloseToGolden(a.data(), a.product(b));

    Mat4 r = Mat4::translate(Vector3(1.f, 1.f, 1.f));
    checkMatrixCloseToGolden(r.data(), a.product(r));

    Mat4 s = Mat4::translate(Vector3(2.f, 2.f, 2.f));
    Mat4 t = r.product(r);
    checkMatrixCloseToGolden(s.data(), t);
}

}

